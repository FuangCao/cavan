package com.cavan.cavanjni;

import android.content.Context;
import android.content.Intent;
import android.util.AttributeSet;

import com.cavan.resource.CavanServicePreference;

public class FtpServicePreference extends CavanServicePreference {

	public FtpServicePreference(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	@Override
	public Intent getServiceIntent(Context context) {
		return new Intent(context, FtpService.class);
	}
}