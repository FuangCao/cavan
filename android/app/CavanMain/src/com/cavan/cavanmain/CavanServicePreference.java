package com.cavan.cavanmain;

import android.content.Context;
import android.content.res.Resources;
import android.os.RemoteException;
import android.preference.EditTextPreference;
import android.text.InputType;
import android.util.AttributeSet;
import android.view.View;

import com.cavan.cavanutils.CavanUtils;

public abstract class CavanServicePreference extends EditTextPreference {

	private ICavanService mService;

	public CavanServicePreference(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public void setService(ICavanService service) {
		mService = service;

		if (service == null) {
			setEnabled(false);
		} else {
			setEnabled(true);
			updateState();
		}
	}

	public ICavanService getService() {
		return mService;
	}

	public void updateSummary(boolean state) {
		Resources resources = getContext().getResources();
		if (resources == null) {
			return;
		}

		StringBuilder builder = new StringBuilder();

		if (state) {
			builder.append(resources.getString(R.string.text_running));
		} else {
			builder.append(resources.getString(R.string.text_stopped));
		}

		builder.append(", ");
		builder.append(resources.getString(R.string.text_port));
		builder.append(": ");
		builder.append(Integer.toString(getPort()));

		setSummary(builder.toString());
	}

	public void updateState() {
		updateSummary(getState());
	}

	public boolean start() {
		if (mService != null) {
			try {
				start(mService);
				return true;
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}

		return false;
	}

	public boolean getState() {
		if (mService != null) {
			try {
				return getState(mService);
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}

		return false;
	}

	public int getPort() {
		if (mService != null) {
			try {
				return getPort(mService);
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}

		return -1;
	}

	public void setPort(int port) {
		if (mService != null) {
			try {
				setPort(mService, port);
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}
	}

	@Override
	protected void onDialogClosed(boolean positiveResult) {
		CavanUtils.logE("onDialogClosed: positiveResult = " + positiveResult);

		if (getState()) {
			return;
		}

		super.onDialogClosed(positiveResult);

		if (positiveResult) {
			try {
				setPort(Integer.parseInt(getText()));
				start();
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	@Override
	protected View onCreateDialogView() {
		CavanUtils.logE("onCreateDialogView");

		getEditText().setInputType(InputType.TYPE_CLASS_NUMBER);

		setDialogMessage(R.string.text_port);
		setText(Integer.toString(getPort()));

		return super.onCreateDialogView();
	}

	protected abstract String getAction();
	protected abstract int getPort(ICavanService service) throws RemoteException;
	protected abstract void setPort(ICavanService service, int port) throws RemoteException;
	protected abstract void start(ICavanService service) throws RemoteException;
	protected abstract boolean getState(ICavanService service) throws RemoteException;
}
