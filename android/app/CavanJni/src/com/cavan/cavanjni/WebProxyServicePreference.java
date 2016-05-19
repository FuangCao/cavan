package com.cavan.cavanjni;

import android.content.Context;
import android.content.Intent;
import android.util.AttributeSet;

public class WebProxyServicePreference extends CavanServicePreference {

	public static final String NAME = "WEB_PROXY";

	public WebProxyServicePreference(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	@Override
	public Intent getServiceIntent(Context context) {
		return new Intent(context, WebProxyService.class);
	}
}