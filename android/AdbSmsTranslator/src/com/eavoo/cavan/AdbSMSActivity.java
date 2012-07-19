package com.eavoo.cavan;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.EditTextPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceActivity;
import android.provider.Settings;
import android.text.InputType;
import android.util.Log;
import android.widget.EditText;

public class AdbSMSActivity extends PreferenceActivity implements OnPreferenceChangeListener
{
	private static final String TAG = "AdbSMSActivity";
	private static final String KEY_SERVICE_ENABLE = "service_enable";
	private static final String KEY_ADB_PORT = "adb_port";
	private static final String KEY_TRANSLATOR_PORT = "translator_port";

	private CheckBoxPreference mCheckBoxPreferenceEnable;
	private ListPreference mListPreferenceAdbPort;
	private EditTextPreference mEditTextPreferenceTranslatorPort;
	
	private String mAdbPort;
	private String mTranslatorPort;
	
	BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver()
	{
		@Override
		public void onReceive(Context context, Intent intent)
		{
			Log.i(TAG, "onReceive: action = " + intent.getAction());
		}
	};

	@Override
	protected void onPause()
	{
		unregisterReceiver(mBroadcastReceiver);
		super.onPause();
	}

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		Log.i(TAG, "onCreate");
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);
		addPreferencesFromResource(R.xml.adb_sms);
		
		IntentFilter filter = new IntentFilter();
		filter.addAction(AdbSMSService.ACTION_SERVICE_RUNNING);
		filter.addAction(AdbSMSService.ACTION_SERVICE_STOPPED);
		filter.addAction(AdbSMSService.ACTION_SERVICE_START_FAILED);
		filter.addAction(AdbSMSService.ACTION_SERVICE_STOP_FAILED);
		registerReceiver(mBroadcastReceiver, filter);
		
		mCheckBoxPreferenceEnable = (CheckBoxPreference) findPreference(KEY_SERVICE_ENABLE);
		mListPreferenceAdbPort = (ListPreference) findPreference(KEY_ADB_PORT);
		mEditTextPreferenceTranslatorPort = (EditTextPreference) findPreference(KEY_TRANSLATOR_PORT);

		setServiceEnable(-1);
		mCheckBoxPreferenceEnable.setOnPreferenceChangeListener(this);

		setAdbPort(null);
		mListPreferenceAdbPort.setOnPreferenceChangeListener(this);

		setTranlatorPort(null);
		EditText editText = mEditTextPreferenceTranslatorPort.getEditText();
		editText.setInputType(InputType.TYPE_CLASS_NUMBER);
		editText.setSingleLine();
		mEditTextPreferenceTranslatorPort.setOnPreferenceChangeListener(this);

		if (mCheckBoxPreferenceEnable.isChecked())
		{
			startService();
		}
	}

	@Override
	protected void onDestroy()
	{
		if (mAdbPort != null)
		{
			Settings.System.putString(getContentResolver(), KEY_ADB_PORT, mAdbPort);
		}

		if (mTranslatorPort != null)
		{
			Settings.System.putString(getContentResolver(), KEY_TRANSLATOR_PORT, mTranslatorPort);
		}

		Settings.System.putInt(getContentResolver(), KEY_SERVICE_ENABLE, mCheckBoxPreferenceEnable.isChecked() ? 1 : 0);

		super.onDestroy();
	}

	private boolean startService()
	{
		Intent intent = new Intent(getApplicationContext(), com.eavoo.cavan.AdbSMSService.class);
		intent.putExtra("adb_port", new Integer(mAdbPort));
		intent.putExtra("translator_port", new Integer(mTranslatorPort));
		Context context = getApplicationContext();
		String pathname = context.getFileStreamPath("eavoo.txt").getPath();
		Log.i(TAG, "path = " + pathname);
		intent.putExtra("pathname", pathname);
		startService(intent);

		return true;
	}

	private boolean stopService()
	{
		Intent intent = new Intent(getApplicationContext(), com.eavoo.cavan.AdbSMSService.class);
		return stopService(intent);
	}
	
	private void setServiceEnable(int enable)
	{
		if (enable < 0)
		{
			enable = Settings.System.getInt(getContentResolver(), KEY_SERVICE_ENABLE, 0);
			mCheckBoxPreferenceEnable.setChecked(enable != 0);
		}
		else if (enable == 0)
		{
			stopService();
			mCheckBoxPreferenceEnable.setSummary(R.string.service_anable);
		}
		else
		{
			startService();
			mCheckBoxPreferenceEnable.setSummary(R.string.service_disable);
		}
	}
	
	private void setAdbPort(String port)
	{
		if (port == null)
		{
			mAdbPort = Settings.System.getString(getContentResolver(), KEY_ADB_PORT);
			if (mAdbPort == null)
			{
				mAdbPort = mListPreferenceAdbPort.getEntryValues()[0].toString();
			}

			mListPreferenceAdbPort.setValue(mAdbPort);
		}
		else
		{
			mAdbPort = port;

			if (mCheckBoxPreferenceEnable.isChecked())
			{
				startService();
			}
		}

		Log.i(TAG, "mAdbPort = " + mAdbPort);
		mListPreferenceAdbPort.setSummary(mAdbPort);
	}

	private void setTranlatorPort(String port)
	{
		if (port == null)
		{
			mTranslatorPort = Settings.System.getString(getContentResolver(), KEY_TRANSLATOR_PORT);
			if (mTranslatorPort == null)
			{
				mTranslatorPort = "8888";
			}

			mEditTextPreferenceTranslatorPort.setText(mTranslatorPort);
		}
		else
		{
			mTranslatorPort = port;
	
			if (mCheckBoxPreferenceEnable.isChecked())
			{
				startService();
			}
		}
		
		Log.i(TAG, "mTranslatorPort = " + mTranslatorPort);
		mEditTextPreferenceTranslatorPort.setSummary(mTranslatorPort);
	}

	@Override
	public boolean onPreferenceChange(Preference preference, Object newValue)
	{
		Intent intent = new Intent(this, com.eavoo.cavan.AdbSMSService.class);

		if (preference.equals(mCheckBoxPreferenceEnable))
		{
			if (newValue.equals(true))
			{
				setServiceEnable(1);
			}
			else
			{
				setServiceEnable(0);
			}

			return true;
		}
		else if (preference.equals(mListPreferenceAdbPort))
		{
			setAdbPort((String) newValue);
			return true;
		}
		else if (preference.equals(mEditTextPreferenceTranslatorPort))
		{
			setTranlatorPort((String) newValue);
			return true;
		}

		return false;
	}
}