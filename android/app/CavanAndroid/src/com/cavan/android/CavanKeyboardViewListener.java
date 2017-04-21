package com.cavan.android;

import android.widget.EditText;

public interface CavanKeyboardViewListener {
	void onStartInput(EditText view);
	void onStopInput(EditText view);
	void onEditTextClick(EditText view);
	void onEditTextLongClick(EditText view);
}