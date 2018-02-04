package com.cavan.accessibility;

import android.inputmethodservice.InputMethodService;

public class CavanInputMethodService extends InputMethodService {

	public static CavanInputMethodService instance;

	@Override
	public void onCreate() {
		super.onCreate();
		instance = this;
	}

	@Override
	public void onDestroy() {
		instance = null;
		super.onDestroy();
	}

	public boolean inputAlipayCode(String code) {
		return false;
	}

	public boolean commitAlipayCode() {
		return false;
	}

}
