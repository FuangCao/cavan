package com.eavoo.touchscreen;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;
import android.preference.EditTextPreference;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.EditText;

public class FirmwarePathPreference extends EditTextPreference implements OnClickListener
{
	private static final String TAG = "FirmwarePathPreference";

	public FirmwarePathPreference(Context context, AttributeSet attrs)
	{
		super(context, attrs);
		Log.i(TAG, "FirmwarePathPreference(Context context, AttributeSet attrs)");
	}

	private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver()
	{
		@Override
		public void onReceive(Context context, Intent intent)
		{
			Log.i(TAG, "action = " + intent.getAction());

			getEditText().setText(intent.getStringExtra("pathname"));
		}
	};

	@Override
	protected void onDialogClosed(boolean positiveResult)
	{
		Log.i(TAG, "onDialogClosed(), positiveResult = " + positiveResult);

		getContext().unregisterReceiver(mBroadcastReceiver);
		super.onDialogClosed(positiveResult);
	}

	@Override
	protected View onCreateDialogView()
	{
		Log.i(TAG, "onCreateDialogView()");
		EditText editText = getEditText();
		editText.setOnClickListener(this);

		IntentFilter filter = new IntentFilter(FileBrowserActivity.ACTION_PATHNAME);
		getContext().registerReceiver(mBroadcastReceiver, filter);

		return super.onCreateDialogView();
	}

	@Override
	public void onClick(View v)
	{
		Log.i(TAG, "onClick(View v)");

		Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(getEditText().getText().toString()));
		intent.setClass(getContext(), FileBrowserActivity.class);
		getContext().startActivity(intent);
	}
}