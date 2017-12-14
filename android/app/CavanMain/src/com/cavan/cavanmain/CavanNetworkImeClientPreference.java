package com.cavan.cavanmain;

import android.content.Context;
import android.content.Intent;
import android.util.AttributeSet;

import com.cavan.resource.CavanTcpClientPreference;

public class CavanNetworkImeClientPreference extends CavanTcpClientPreference {

	public CavanNetworkImeClientPreference(Context context) {
		super(context);
	}

	public CavanNetworkImeClientPreference(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public CavanNetworkImeClientPreference(Context context, AttributeSet attrs, int defStyleAttr) {
		super(context, attrs, defStyleAttr);
	}

	public CavanNetworkImeClientPreference(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
		super(context, attrs, defStyleAttr, defStyleRes);
	}

	@Override
	public Intent getServiceIntent() {
		return CavanNetworkImeConnService.getIntent(getContext());
	}
}
