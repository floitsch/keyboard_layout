// Copyright (C) 2026 Toit contributors.
// SPDX-License-Identifier: Unlicense

#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_DEVICE "/dev/input/by-id/usb-1ea7_2.4G_Mouse-if01-event-mouse"
#define DEFAULT_MOTION_THRESHOLD 12U
#define BACK_SCAN_CODE 0x90004U
#define BITS_PER_LONG (sizeof(unsigned long) * 8U)
#define NBITS(max) (((max) / BITS_PER_LONG) + 1U)

static volatile sig_atomic_t stop_requested;

enum back_gesture {
  BACK_IDLE,
  BACK_PENDING,
  BACK_STARTED_SCROLL,
  BACK_EXITING_SCROLL,
};

struct gesture_state {
  enum back_gesture back;
  bool meta_locked;
  bool scroll_locked;
  uint64_t motion;
};

struct actions {
  int meta;
  int scroll;
};

static struct actions no_actions(void) {
  return (struct actions){.meta = -1, .scroll = -1};
}

static struct actions handle_back(struct gesture_state *state, int value) {
  struct actions actions = no_actions();

  if (value == 1) {
    state->motion = 0;
    if (state->scroll_locked) {
      state->scroll_locked = false;
      state->back = BACK_EXITING_SCROLL;
      actions.scroll = 0;
    } else {
      state->back = BACK_PENDING;
    }
  } else if (value == 0) {
    if (state->back == BACK_PENDING) {
      state->meta_locked = !state->meta_locked;
      actions.meta = state->meta_locked;
    }
    state->back = BACK_IDLE;
    state->motion = 0;
  }

  return actions;
}

static struct actions handle_motion(
    struct gesture_state *state, int value, unsigned threshold) {
  struct actions actions = no_actions();
  if (state->back != BACK_PENDING) return actions;

  int64_t signed_value = value;
  state->motion += signed_value < 0 ? (uint64_t)-signed_value
                                    : (uint64_t)signed_value;
  if (state->motion < threshold) return actions;

  if (state->meta_locked) {
    state->meta_locked = false;
    actions.meta = 0;
  }
  state->scroll_locked = true;
  state->back = BACK_STARTED_SCROLL;
  actions.scroll = 1;
  return actions;
}

static int expect_actions(
    const char *label, struct actions actual, int meta, int scroll) {
  if (actual.meta == meta && actual.scroll == scroll) return 0;
  fprintf(stderr,
          "self-test %s failed: got meta=%d scroll=%d, expected meta=%d "
          "scroll=%d\n",
          label, actual.meta, actual.scroll, meta, scroll);
  return -1;
}

static int self_test(void) {
  struct gesture_state state = {0};
  struct actions actions;

  actions = handle_back(&state, 1);
  if (expect_actions("tap-down", actions, -1, -1) < 0) return 1;
  actions = handle_back(&state, 0);
  if (expect_actions("tap-locks-meta", actions, 1, -1) < 0) return 1;

  handle_back(&state, 1);
  actions = handle_motion(&state, 11, 12);
  if (expect_actions("motion-below-threshold", actions, -1, -1) < 0) return 1;
  actions = handle_motion(&state, -1, 12);
  if (expect_actions("motion-starts-scroll", actions, 0, 1) < 0) return 1;
  actions = handle_back(&state, 0);
  if (expect_actions("release-keeps-scroll", actions, -1, -1) < 0) return 1;

  actions = handle_back(&state, 1);
  if (expect_actions("click-stops-scroll", actions, -1, 0) < 0) return 1;
  actions = handle_back(&state, 0);
  if (expect_actions("stop-click-release", actions, -1, -1) < 0) return 1;

  handle_back(&state, 1);
  handle_motion(&state, 3, 12);
  actions = handle_back(&state, 0);
  if (expect_actions("jitter-is-still-tap", actions, 1, -1) < 0) return 1;
  handle_back(&state, 1);
  actions = handle_back(&state, 0);
  if (expect_actions("second-tap-unlocks-meta", actions, 0, -1) < 0) return 1;

  puts("mouse-meta-toggle self-test passed");
  return 0;
}

static bool bit_is_set(const unsigned long *bits, unsigned bit) {
  return (bits[bit / BITS_PER_LONG] & (1UL << (bit % BITS_PER_LONG))) != 0;
}

static int write_event(int fd, unsigned type, unsigned code, int value) {
  struct input_event event = {.type = type, .code = code, .value = value};
  const char *data = (const char *)&event;
  size_t remaining = sizeof(event);
  while (remaining > 0) {
    ssize_t written = write(fd, data, remaining);
    if (written < 0) {
      if (errno == EINTR) continue;
      return -1;
    }
    data += written;
    remaining -= (size_t)written;
  }
  return 0;
}

static int emit_key(int fd, unsigned code, int value) {
  if (write_event(fd, EV_KEY, code, value) < 0) return -1;
  return write_event(fd, EV_SYN, SYN_REPORT, 0);
}

static int apply_actions(
    int mouse_fd, int keyboard_fd, const struct actions *actions) {
  if (actions->meta >= 0) {
    if (emit_key(keyboard_fd, KEY_LEFTMETA, actions->meta) < 0) return -1;
    fprintf(stderr, "Meta %s\n", actions->meta ? "locked" : "released");
  }
  if (actions->scroll >= 0) {
    if (emit_key(mouse_fd, BTN_SIDE, actions->scroll) < 0) return -1;
    fprintf(stderr, "scroll %s\n", actions->scroll ? "locked" : "released");
  }
  return 0;
}

static int open_uinput(void) {
  int fd = open("/dev/uinput", O_WRONLY | O_CLOEXEC);
  if (fd < 0) perror("open /dev/uinput");
  return fd;
}

static int create_virtual_mouse(int input_fd) {
  unsigned long key_bits[NBITS(KEY_MAX)] = {0};
  unsigned long rel_bits[NBITS(REL_MAX)] = {0};
  unsigned long prop_bits[NBITS(INPUT_PROP_MAX)] = {0};
  struct input_id id = {0};
  char name[UINPUT_MAX_NAME_SIZE] = "2.4G Mouse";
  int fd = -1;

  if (ioctl(input_fd, EVIOCGID, &id) < 0) {
    perror("EVIOCGID");
    return -1;
  }
  if (ioctl(input_fd, EVIOCGNAME(sizeof(name)), name) < 0) {
    strcpy(name, "2.4G Mouse");
  }
  if (ioctl(input_fd, EVIOCGBIT(EV_KEY, sizeof(key_bits)), key_bits) < 0 ||
      ioctl(input_fd, EVIOCGBIT(EV_REL, sizeof(rel_bits)), rel_bits) < 0) {
    perror("EVIOCGBIT");
    return -1;
  }
  if (ioctl(input_fd, EVIOCGPROP(sizeof(prop_bits)), prop_bits) < 0) {
    memset(prop_bits, 0, sizeof(prop_bits));
  }

  fd = open_uinput();
  if (fd < 0) return -1;
  if (ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0 ||
      ioctl(fd, UI_SET_EVBIT, EV_REL) < 0) {
    perror("UI_SET_EVBIT");
    goto error;
  }

  for (unsigned code = BTN_MISC; code <= KEY_MAX; ++code) {
    if (bit_is_set(key_bits, code) && ioctl(fd, UI_SET_KEYBIT, code) < 0) {
      perror("UI_SET_KEYBIT");
      goto error;
    }
  }
  if (ioctl(fd, UI_SET_KEYBIT, BTN_SIDE) < 0 ||
      ioctl(fd, UI_SET_KEYBIT, BTN_EXTRA) < 0) {
    perror("UI_SET_KEYBIT side buttons");
    goto error;
  }
  for (unsigned code = 0; code <= REL_MAX; ++code) {
    if (bit_is_set(rel_bits, code) && ioctl(fd, UI_SET_RELBIT, code) < 0) {
      perror("UI_SET_RELBIT");
      goto error;
    }
  }
  for (unsigned code = 0; code <= INPUT_PROP_MAX; ++code) {
    if (bit_is_set(prop_bits, code) && ioctl(fd, UI_SET_PROPBIT, code) < 0) {
      perror("UI_SET_PROPBIT");
      goto error;
    }
  }

  struct uinput_setup setup = {.id = id};
  snprintf(setup.name, sizeof(setup.name), "%s", name);
  if (ioctl(fd, UI_DEV_SETUP, &setup) < 0 ||
      ioctl(fd, UI_DEV_CREATE) < 0) {
    perror("create virtual mouse");
    goto error;
  }
  return fd;

error:
  close(fd);
  return -1;
}

static int create_virtual_keyboard(const struct input_id *mouse_id) {
  int fd = open_uinput();
  if (fd < 0) return -1;
  if (ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0 ||
      ioctl(fd, UI_SET_KEYBIT, KEY_LEFTMETA) < 0) {
    perror("configure virtual keyboard");
    close(fd);
    return -1;
  }

  struct uinput_setup setup = {.id = *mouse_id};
  snprintf(setup.name, sizeof(setup.name), "2.4G Mouse Meta Toggle");
  if (ioctl(fd, UI_DEV_SETUP, &setup) < 0 ||
      ioctl(fd, UI_DEV_CREATE) < 0) {
    perror("create virtual keyboard");
    close(fd);
    return -1;
  }
  return fd;
}

static void destroy_virtual_device(int fd) {
  if (fd < 0) return;
  if (ioctl(fd, UI_DEV_DESTROY) < 0 && errno != ENODEV) {
    perror("UI_DEV_DESTROY");
  }
  close(fd);
}

static int run_proxy(const char *device, unsigned threshold) {
  struct gesture_state state = {0};
  struct input_id mouse_id = {0};
  int input_fd = -1;
  int mouse_fd = -1;
  int keyboard_fd = -1;
  int result = -1;
  uint32_t last_scan = 0;

  input_fd = open(device, O_RDONLY | O_CLOEXEC);
  if (input_fd < 0) return -1;
  if (ioctl(input_fd, EVIOCGRAB, 1) < 0) {
    perror("EVIOCGRAB");
    goto cleanup;
  }
  if (ioctl(input_fd, EVIOCGID, &mouse_id) < 0) {
    perror("EVIOCGID");
    goto cleanup;
  }

  mouse_fd = create_virtual_mouse(input_fd);
  if (mouse_fd < 0) goto cleanup;
  keyboard_fd = create_virtual_keyboard(&mouse_id);
  if (keyboard_fd < 0) goto cleanup;
  usleep(100000);
  fprintf(stderr, "proxying %s with motion threshold %u\n", device, threshold);

  while (!stop_requested) {
    struct input_event events[64];
    ssize_t size = read(input_fd, events, sizeof(events));
    if (size < 0) {
      if (errno == EINTR && stop_requested) {
        result = 0;
        break;
      }
      if (errno == EINTR) continue;
      perror("read mouse events");
      break;
    }
    if (size == 0) break;
    if ((size_t)size % sizeof(events[0]) != 0) {
      fprintf(stderr, "short mouse event read\n");
      break;
    }

    size_t count = (size_t)size / sizeof(events[0]);
    for (size_t i = 0; i < count; ++i) {
      struct input_event *event = &events[i];
      struct actions actions = no_actions();

      if (event->type == EV_MSC && event->code == MSC_SCAN) {
        last_scan = (uint32_t)event->value;
        continue;
      }
      if (event->type == EV_SYN && event->code == SYN_DROPPED) {
        fprintf(stderr, "input event buffer overrun; reconnecting\n");
        goto cleanup;
      }
      if (event->type == EV_KEY) {
        bool is_back = event->code == BTN_SIDE || last_scan == BACK_SCAN_CODE;
        last_scan = 0;
        if (is_back) {
          actions = handle_back(&state, event->value);
          if (apply_actions(mouse_fd, keyboard_fd, &actions) < 0) {
            perror("emit gesture action");
            goto cleanup;
          }
          continue;
        }
      }
      if (event->type == EV_REL &&
          (event->code == REL_X || event->code == REL_Y)) {
        actions = handle_motion(&state, event->value, threshold);
        if (apply_actions(mouse_fd, keyboard_fd, &actions) < 0) {
          perror("emit gesture action");
          goto cleanup;
        }
      }
      if (event->type == EV_MSC) continue;
      if (write(mouse_fd, event, sizeof(*event)) != (ssize_t)sizeof(*event)) {
        perror("forward mouse event");
        goto cleanup;
      }
      if (event->type == EV_SYN) last_scan = 0;
    }
  }

cleanup:
  if (state.meta_locked && keyboard_fd >= 0) {
    emit_key(keyboard_fd, KEY_LEFTMETA, 0);
  }
  if (state.scroll_locked && mouse_fd >= 0) {
    emit_key(mouse_fd, BTN_SIDE, 0);
  }
  if (keyboard_fd >= 0 || mouse_fd >= 0) usleep(20000);
  destroy_virtual_device(keyboard_fd);
  destroy_virtual_device(mouse_fd);
  if (input_fd >= 0) {
    ioctl(input_fd, EVIOCGRAB, 0);
    close(input_fd);
  }
  return result;
}

static void handle_signal(int signal_number) {
  (void)signal_number;
  stop_requested = 1;
}

static void wait_before_retry(void) {
  struct timespec delay = {.tv_sec = 1};
  while (!stop_requested && nanosleep(&delay, &delay) < 0 && errno == EINTR) {
  }
}

int main(int argc, char **argv) {
  const char *device = DEFAULT_DEVICE;
  unsigned threshold = DEFAULT_MOTION_THRESHOLD;

  if (argc == 2 && strcmp(argv[1], "--self-test") == 0) return self_test();
  if (argc > 3) {
    fprintf(stderr, "usage: %s [device [motion-threshold]]\n", argv[0]);
    return 2;
  }
  if (argc >= 2) device = argv[1];
  if (argc == 3) {
    char *end = NULL;
    unsigned long parsed = strtoul(argv[2], &end, 10);
    if (*argv[2] == '\0' || *end != '\0' || parsed == 0 || parsed > 10000) {
      fprintf(stderr, "invalid motion threshold: %s\n", argv[2]);
      return 2;
    }
    threshold = (unsigned)parsed;
  }

  struct sigaction action = {.sa_handler = handle_signal};
  sigemptyset(&action.sa_mask);
  sigaction(SIGINT, &action, NULL);
  sigaction(SIGTERM, &action, NULL);

  while (!stop_requested) {
    run_proxy(device, threshold);
    if (!stop_requested) wait_before_retry();
  }
  return 0;
}
