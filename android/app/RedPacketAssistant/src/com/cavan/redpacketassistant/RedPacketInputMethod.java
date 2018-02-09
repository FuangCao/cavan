package com.cavan.redpacketassistant;

import com.cavan.accessibility.CavanInputMethodService;
import com.cavan.android.CavanAndroid;

public class RedPacketInputMethod extends CavanInputMethodService {

	@Override
	public void onCreate() {
		super.onCreate();
		CavanAndroid.pLog();
	}

	@Override
	public void onDestroy() {
		CavanAndroid.pLog();
		super.onDestroy();
	}

}
