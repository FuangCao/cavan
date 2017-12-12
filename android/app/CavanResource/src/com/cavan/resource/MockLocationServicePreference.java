package com.cavan.resource;

import android.content.Context;
import android.content.Intent;
import android.util.AttributeSet;

import com.cavan.service.MockLocationService;

public class MockLocationServicePreference extends CavanServicePreference {

	public static final String NAME = "MOCK_LOCATION";

	public MockLocationServicePreference(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	@Override
	public Intent getServiceIntent(Context context) {
		return new Intent(context, MockLocationService.class);
	}
}