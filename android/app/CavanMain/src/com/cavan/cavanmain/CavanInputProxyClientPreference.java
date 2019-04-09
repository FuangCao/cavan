package com.cavan.cavanmain;

import android.content.Context;
import android.content.Intent;
import android.util.AttributeSet;

import com.cavan.resource.CavanTcpClientPreference;

public class CavanInputProxyClientPreference extends CavanTcpClientPreference {

	public CavanInputProxyClientPreference(Context context) {
		super(context);
	}

	public CavanInputProxyClientPreference(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public CavanInputProxyClientPreference(Context context, AttributeSet attrs, int defStyleAttr) {
		super(context, attrs, defStyleAttr);
	}

	public CavanInputProxyClientPreference(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
		super(context, attrs, defStyleAttr, defStyleRes);
	}

	@Override
	public Intent getServiceIntent() {
		return CavanInputProxyConnService.getIntent(getContext());
	}
}
