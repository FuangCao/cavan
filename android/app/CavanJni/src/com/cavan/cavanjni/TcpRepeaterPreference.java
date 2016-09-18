package com.cavan.cavanjni;

import android.content.Context;
import android.content.Intent;
import android.util.AttributeSet;

public class TcpRepeaterPreference extends CavanServicePreference {

	public TcpRepeaterPreference(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	@Override
	public Intent getServiceIntent(Context context) {
		return new Intent(context, TcpRepeater.class);
	}
}
