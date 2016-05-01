package com.cavan.cavanmain;

import android.content.Context;
import android.os.RemoteException;
import android.preference.Preference;
import android.util.AttributeSet;

public abstract class CavanServicePreference extends Preference {

	private ICavanService mService;

	public CavanServicePreference(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public void setService(ICavanService service) {
		mService = service;
		updateState();
	}

	public ICavanService getService() {
		return mService;
	}

	public void updateSummary(boolean state) {
		if (state) {
			setSummary(R.string.text_running);
		} else {
			setSummary(R.string.text_stopped);
		}
	}

	public void updateState() {
		updateSummary(getState());
	}

	public boolean start() {
		if (mService == null) {
			return false;
		}

		try {
			start(mService);
			return true;
		} catch (RemoteException e) {
			e.printStackTrace();
		}

		return false;
	}

	public boolean getState() {
		if (mService == null) {
			return false;
		}

		try {
			return getState(mService);
		} catch (RemoteException e) {
			e.printStackTrace();
		}

		return false;
	}

	protected abstract String getAction();
	protected abstract void start(ICavanService service) throws RemoteException;
	protected abstract boolean getState(ICavanService service) throws RemoteException;
}
