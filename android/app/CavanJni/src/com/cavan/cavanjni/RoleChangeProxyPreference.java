package com.cavan.cavanjni;

import android.content.Context;
import android.content.Intent;
import android.util.AttributeSet;

import com.cavan.resource.CavanCommandServicePreference;

public class RoleChangeProxyPreference extends CavanCommandServicePreference {

	public RoleChangeProxyPreference(Context context) {
		super(context);
	}

	public RoleChangeProxyPreference(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public RoleChangeProxyPreference(Context context, AttributeSet attrs, int defStyleAttr) {
		super(context, attrs, defStyleAttr);
	}

	public RoleChangeProxyPreference(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
		super(context, attrs, defStyleAttr, defStyleRes);
	}

	@Override
	public Intent getServiceIntent(Context context) {
		return new Intent(context, RoleChangeProxy.class);
	}
}
