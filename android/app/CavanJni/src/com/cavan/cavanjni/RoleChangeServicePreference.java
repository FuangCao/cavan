package com.cavan.cavanjni;

import android.content.Context;
import android.content.Intent;
import android.util.AttributeSet;

import com.cavan.resource.CavanCommandServicePreference;

public class RoleChangeServicePreference extends CavanCommandServicePreference {

	public RoleChangeServicePreference(Context context) {
		super(context);
	}

	public RoleChangeServicePreference(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public RoleChangeServicePreference(Context context, AttributeSet attrs, int defStyleAttr) {
		super(context, attrs, defStyleAttr);
	}

	public RoleChangeServicePreference(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
		super(context, attrs, defStyleAttr, defStyleRes);
	}

	@Override
	public Intent getServiceIntent(Context context) {
		return new Intent(context, RoleChangeService.class);
	}
}
