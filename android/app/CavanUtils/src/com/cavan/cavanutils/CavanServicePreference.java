package com.cavan.cavanutils;

import java.util.HashMap;
import java.util.concurrent.ExecutionException;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.res.Resources;
import android.os.IBinder;
import android.os.RemoteException;
import android.preference.EditTextPreference;
import android.text.InputType;
import android.util.AttributeSet;
import android.view.View;

public class CavanServicePreference extends EditTextPreference {

	private static HashMap<String, Class<?>> mHashMap = new HashMap<String, Class<?>>();

	static {
		mHashMap.put("ftp_service", FtpService.class);
		mHashMap.put("tcp_dd_service", TcpDdService.class);
		mHashMap.put("web_proxy_service", WebProxyService.class);
	}

	private Class<?> mServiceClass;
	private ICavanService mService;
	private ServiceConnection mConnection = new ServiceConnection() {

		@Override
		public void onServiceDisconnected(ComponentName arg0) {
			CavanUtils.logE("onServiceDisconnected: " + arg0);

			try {
				getContext().unregisterReceiver(mReceiver);
			} catch (Exception e) {
				e.printStackTrace();
			}

			mService = null;
			setEnabled(false);
		}

		@Override
		public void onServiceConnected(ComponentName arg0, IBinder arg1) {
			CavanUtils.logE("onServiceConnected: " + arg0);

			mService = ICavanService.Stub.asInterface(arg1);

			try {
				IntentFilter filter = new IntentFilter(mService.getAction());
				getContext().registerReceiver(mReceiver, filter);
			} catch (RemoteException e) {
				e.printStackTrace();
			}

			setEnabled(true);
			updateSummary(getState());
		}
	};

	private BroadcastReceiver mReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			boolean state = intent.getBooleanExtra("state", false);
			updateSummary(state);
		}
	};

	public CavanServicePreference(Context context, AttributeSet attrs) throws ExecutionException {
		super(context, attrs);

		mServiceClass = mHashMap.get(getKey());
		if (mServiceClass == null) {
			throw new ExecutionException("Invalid service: " + getKey(), new Throwable());
		}

		updateSummary(false);

		Intent service = new Intent(context, mServiceClass);
		context.startService(service);
		context.bindService(service, mConnection, 0);
	}

	@Override
	protected void onDialogClosed(boolean positiveResult) {
		if (getState()) {
			return;
		}

		super.onDialogClosed(positiveResult);

		if (positiveResult) {
			start(Integer.parseInt(getText()));
		}
	}

	@Override
	protected View onCreateDialogView() {
		setDialogMessage(R.string.text_port);
		setText(Integer.toString(getPort()));
		getEditText().setInputType(InputType.TYPE_CLASS_NUMBER);

		return super.onCreateDialogView();
	}

	public boolean getState() {
		if (mService != null) {
			try {
				return mService.getState();
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}

		return false;
	}

	public void start(int port) {
		if (mService != null) {
			try {
				mService.start(port);
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}
	}

	public int getPort() {
		if (mService != null) {
			try {
				return mService.getPort();
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}

		return 0;
	}

	private void updateSummary(boolean state) {

		if (mService == null) {
			setSummary(R.string.text_service_disconnected);
		} else {
			Resources resources = getContext().getResources();
			if (resources == null) {
				setSummary(R.string.text_service_connected);
			} else {
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
		}
	}
}