package com.cavan.cavanjni;

import android.content.Context;
import android.content.Intent;
import android.util.AttributeSet;

import com.cavan.resource.CavanServicePreference;

public class HttpServicePreference extends CavanServicePreference {

	public HttpServicePreference(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	@Override
	public Intent getServiceIntent(Context context) {
		return new Intent(context, HttpService.class);
	}
}