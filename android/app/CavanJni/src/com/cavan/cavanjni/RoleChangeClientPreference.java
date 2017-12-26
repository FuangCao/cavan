package com.cavan.cavanjni;

import android.content.Context;
import android.content.Intent;
import android.util.AttributeSet;

import com.cavan.resource.CavanCommandServicePreference;

public class RoleChangeClientPreference extends CavanCommandServicePreference {

	public RoleChangeClientPreference(Context context) {
		super(context);
	}

	public RoleChangeClientPreference(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public RoleChangeClientPreference(Context context, AttributeSet attrs, int defStyleAttr) {
		super(context, attrs, defStyleAttr);
	}

	public RoleChangeClientPreference(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
		super(context, attrs, defStyleAttr, defStyleRes);
	}

	@Override
	public Intent getServiceIntent(Context context) {
		return new Intent(context, RoleChangeClient.class);
	}
}
