package com.cavan.cavanmain;

import android.inputmethodservice.InputMethodService;
import android.view.View;
import android.widget.Button;

public class CavanInputMethod extends InputMethodService {

	@Override
	public View onCreateInputView() {
		Button button = new Button(this);
		button.setText("11223344");
		return button;
	}
}
