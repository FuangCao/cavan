package com.cavan.cavanutils;

import android.content.Context;
import android.content.Intent;
import android.util.AttributeSet;

public class TcpDdServicePreference extends CavanServicePreference {

	public TcpDdServicePreference(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	@Override
	public Intent getServiceIntent(Context context) {
		return new Intent(context, TcpDdService.class);
	}
}