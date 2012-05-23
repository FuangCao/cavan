package com.eavoo.touchscreen;

import android.content.Context;
import android.preference.EditTextPreference;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.EditText;

public class FirmwarePathPreference extends EditTextPreference implements OnClickListener
{
	private static final String TAG = "FirmwarePathPreference";

	public FirmwarePathPreference(Context context)
	{
		super(context);
		Log.i(TAG, "FirmwarePathPreference(Context context)");
	}

	public FirmwarePathPreference(Context context, AttributeSet attrs, int defStyle)
	{
		super(context, attrs, defStyle);
		Log.i(TAG, "FirmwarePathPreference(Context context, AttributeSet attrs, int defStyle)");
	}

	public FirmwarePathPreference(Context context, AttributeSet attrs)
	{
		super(context, attrs);
		Log.i(TAG, "FirmwarePathPreference(Context context, AttributeSet attrs)");
	}

	@Override
	protected void onClick()
	{
		Log.i(TAG, "onClick()");
		
		EditText editText = getEditText();
		editText.setOnClickListener(this);

		super.onClick();
	}

	@Override
	public void onClick(View v)
	{
		Log.i(TAG, "onClick(View v)");
	}
}