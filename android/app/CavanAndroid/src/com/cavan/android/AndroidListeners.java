package com.cavan.android;

import android.widget.EditText;

public class AndroidListeners {

	public interface CavanKeyboardViewListener {
		void onStartInput(EditText view);
		void onStopInput(EditText view);
		void onEditTextClick(EditText view);
		void onEditTextLongClick(EditText view);
	}

	public interface CavanBusyLockListener {
		void onBusyLockAcquired(Object owner);
		void onBusyLockReleased(Object owner);
	}
}
