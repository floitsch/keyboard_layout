import ctypes as C
import pathlib
import re
import tempfile

# Requires Python 3, libxkbcommon0, xkb-data and Linux input headers.

repo = pathlib.Path(__file__).resolve().parent.parent
lib = C.CDLL('libxkbcommon.so.0')
class Names(C.Structure):
    _fields_ = [(key, C.c_char_p) for key in ('rules', 'model', 'layout', 'variant', 'options')]
for name, args, result in [
    ('xkb_context_new', [C.c_int], C.c_void_p),
    ('xkb_context_include_path_append', [C.c_void_p, C.c_char_p], C.c_int),
    ('xkb_keymap_new_from_names', [C.c_void_p, C.POINTER(Names), C.c_int], C.c_void_p),
    ('xkb_keymap_key_get_syms_by_level', [C.c_void_p, C.c_uint, C.c_uint, C.c_uint, C.POINTER(C.POINTER(C.c_uint))], C.c_int),
]:
    fn = getattr(lib, name)
    fn.argtypes, fn.restype = args, result
context = lib.xkb_context_new(1)  # Disable defaults to avoid user overrides.
temporary = tempfile.TemporaryDirectory(prefix='shift-layout-test-')
stage = pathlib.Path(temporary.name)
(stage / 'symbols').mkdir(parents=True, exist_ok=True)
for layout in ('us_shifted', 'dv_shifted'):
    (stage / 'symbols' / layout).write_bytes((repo / layout).read_bytes())
assert lib.xkb_context_include_path_append(context, str(stage).encode())
system_root = pathlib.Path('/usr/share/xkeyboard-config-2')
if not system_root.is_dir():
    system_root = pathlib.Path('/usr/share/X11/xkb')
assert lib.xkb_context_include_path_append(context, str(system_root).encode())
keymaps = {}
for layout in ('us_shifted', 'dv_shifted'):
    names = Names(b'evdev', b'pc105', layout.encode(), b'', b'')
    keymaps[layout] = lib.xkb_keymap_new_from_names(context, C.byref(names), 0)
    assert keymaps[layout], layout
temporary.cleanup()
codes = {name.lower(): int(number) for name, number in re.findall(r'^#define KEY_(\w+)\s+(\d+)\b', pathlib.Path('/usr/include/linux/input-event-codes.h').read_text(), re.M)}
def remaps(filename):
    return {int(scan, 16): codes[key] for scan, key in re.findall(r'^ KEYBOARD_KEY_([0-9a-f]+)=(\w+)', (repo / filename).read_text(), re.M) if int(scan, 16) < 256}
base = remaps('90-custom-keyboard.hwdb')
base.update(remaps('90-shift-layout-dell-precision-5490.hwdb'))
translated = base | remaps('91-shift-layout-dell-dvorak.hwdb')
def symbols(layout, code, level):
    syms = C.POINTER(C.c_uint)()
    count = lib.xkb_keymap_key_get_syms_by_level(keymaps[layout], code + 8, 0, level, C.byref(syms))
    return tuple(syms[i] for i in range(count))
# Every ordinary AT key, including the custom modifiers and punctuation, must
# match the existing Dell remaps + dv_shifted for both Shift levels.
for scan in range(1, 89):
    for level in (0, 1):
        expected = symbols('dv_shifted', base.get(scan, scan), level)
        actual = symbols('us_shifted', translated.get(scan, scan), level)
        assert actual == expected, (hex(scan), level, actual, expected)
for scan, plain, shifted in [(2, '!', '1'), (10, '<', '9'), (11, '>', '0'), (51, ',', '('), (52, '.', ')'), (16, 'q', 'Q')]:
    for level, expected in enumerate((plain, shifted)):
        assert symbols('us_shifted', scan, level) == (ord(expected),)
print('PASS: 176 internal key/Shift combinations match shifted Dvorak; external shifted-US punctuation and letters match.')
