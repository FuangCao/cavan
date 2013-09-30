package com.cavan.touchscreen;

import android.content.Context;
import android.preference.Preference;
import android.view.View;

public class FwPreference extends Preference {
	private String mPathname;

	public FwPreference(Context context, String pathname) {
		super(context);
		mPathname = pathname;
	}

	@Override
	protected void onBindView(View view) {
		setTitle(mPathname);
		super.onBindView(view);
	}
}