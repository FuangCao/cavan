package com.cavan.android;

import java.io.File;
import java.util.HashMap;

import android.annotation.SuppressLint;
import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceCategory;
import android.util.AttributeSet;

@SuppressLint("HandlerLeak")
public class PowerSupplyPreferenceCategory extends PreferenceCategory implements OnPreferenceClickListener {

	private static final int EVENT_UPDATE = 1;
	private static final int EVENT_UPDATE_AUTO = 2;

	private long mAutoUpdate;
	private PowerSupply mPowerSupply;
	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case EVENT_UPDATE_AUTO:
				if (mAutoUpdate > 0) {
					mHandler.removeMessages(EVENT_UPDATE_AUTO);
					mHandler.sendEmptyMessageDelayed(EVENT_UPDATE_AUTO, mAutoUpdate);
				} else {
					break;
				}
			case EVENT_UPDATE:
				update();
				break;
			}
		}
	};

	public boolean init(File dir) {
		mPowerSupply = new PowerSupply(dir);
		String type = mPowerSupply.readType();
		if (type == null) {
			return false;
		}

		setTitle(type);

		HashMap<String, String> props = mPowerSupply.readProps();
		if (props == null) {
			return false;
		}

		for (String key : props.keySet()) {
			Preference preference = new Preference(getContext());
			preference.setKey(key);

			if (key.startsWith("POWER_SUPPLY_")) {
				preference.setTitle(key.substring(13));
			} else {
				preference.setTitle(key);
			}

			preference.setSummary(props.get(key));
			preference.setOnPreferenceClickListener(this);
			addPreference(preference);
		}

		return true;
	}

	public boolean init(String pathname) {
		return init(new File(pathname));
	}

	public boolean update() {
		HashMap<String, String> props = mPowerSupply.readProps();
		if (props == null) {
			return false;
		}

		for (int i = getPreferenceCount() - 1; i >= 0; i--) {
			Preference preference = getPreference(i);
			if (preference == null) {
				return false;
			}

			String value = props.get(preference.getKey());
			if (value == null) {
				return false;
			}

			preference.setSummary(value);
		}

		return true;
	}

	public void setAutoUpdate(long delay) {
		mAutoUpdate = delay;

		if (delay > 0) {
			mHandler.sendEmptyMessage(EVENT_UPDATE_AUTO);
		}
	}

	public PowerSupplyPreferenceCategory(Context context, AttributeSet attrs, int defStyleAttr) {
		super(context, attrs, defStyleAttr);
	}

	public PowerSupplyPreferenceCategory(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public PowerSupplyPreferenceCategory(Context context) {
		super(context);
	}

	@Override
	public boolean onPreferenceClick(Preference preference) {
		update();
		return true;
	}
}
