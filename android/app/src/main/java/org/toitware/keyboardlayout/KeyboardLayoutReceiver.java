package org.toitware.keyboardlayout;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

/** Marker receiver used by Android to discover the bundled physical-keyboard layouts. */
public final class KeyboardLayoutReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        // Android reads the receiver's KEYBOARD_LAYOUTS metadata; no runtime work is needed.
    }
}
