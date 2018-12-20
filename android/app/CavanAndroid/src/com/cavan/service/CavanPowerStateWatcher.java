package com.cavan.service;

import java.util.HashSet;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;

import com.cavan.android.CavanAndroid;

public class CavanPowerStateWatcher {

	private HashSet<CavanPowerStateListener> mListeners = new HashSet<CavanPowerStateListener>();
	private boolean mUserPresent = true;
	private boolean mScreenOn = true;

	private BroadcastReceiver mReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			onBroadcastReceive(intent);
		}
	};

	public boolean onBroadcastReceive(Intent intent) {
		String action = intent.getAction();
		CavanAndroid.dLog("action = " + action);

		switch (action) {
		case Intent.ACTION_SCREEN_ON:
			onScreenOn(intent);
			break;

		case Intent.ACTION_SCREEN_OFF:
			onScreenOff(intent);
			break;

		case Intent.ACTION_USER_PRESENT:
			onUserPresent(intent);
			break;

		case Intent.ACTION_CLOSE_SYSTEM_DIALOGS:
			onCloseSystemDialogs(intent);
			break;

		default:
			return false;
		}

		return true;
	}

	public boolean isUserPresent() {
		synchronized (mListeners) {
			return mUserPresent;
		}
	}

	public boolean isScreenOn() {
		synchronized (mListeners) {
			return mScreenOn;
		}
	}

	protected void onCloseSystemDialogs(Intent intent) {
		String reason = intent.getStringExtra("reason");

		synchronized (mListeners) {
			for (CavanPowerStateListener listener : mListeners) {
				listener.onCloseSystemDialogs(reason);
			}
		}
	}

	protected void onUserPresent(Intent intent) {
		synchronized (mListeners) {
			mUserPresent = true;

			for (CavanPowerStateListener listener : mListeners) {
				listener.onUserPresent();
			}
		}
	}

	protected void onScreenOff(Intent intent) {
		synchronized (mListeners) {
			mScreenOn = false;
			mUserPresent = false;

			for (CavanPowerStateListener listener : mListeners) {
				listener.onScreenOff();
			}
		}
	}

	protected void onScreenOn(Intent intent) {
		synchronized (mListeners) {
			mScreenOn = true;

			for (CavanPowerStateListener listener : mListeners) {
				listener.onScreenOn();
			}
		}
	}

	public boolean register(CavanPowerStateListener listener) {
		synchronized (mListeners) {
			if (mListeners.add(listener)) {
				if (mUserPresent) {
					listener.onUserPresent();
				} else if (mScreenOn) {
					listener.onScreenOn();
				} else {
					listener.onScreenOff();
				}

				return true;
			}
		}

		return false;
	}

	public boolean unregister(CavanPowerStateListener listener) {
		synchronized (mListeners) {
			return mListeners.remove(listener);
		}
	}

	public Intent start(Context context) {
		IntentFilter filter = new IntentFilter();
		filter.addAction(Intent.ACTION_SCREEN_ON);
		filter.addAction(Intent.ACTION_SCREEN_OFF);
		filter.addAction(Intent.ACTION_USER_PRESENT);
		filter.addAction(Intent.ACTION_CLOSE_SYSTEM_DIALOGS);
		filter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);

		return context.registerReceiver(mReceiver, filter );
	}

	public void stop(Context context) {
		context.unregisterReceiver(mReceiver);
	}
}
