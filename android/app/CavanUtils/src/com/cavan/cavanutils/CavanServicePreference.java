package com.cavan.cavanutils;

import java.util.HashMap;
import java.util.concurrent.ExecutionException;

import android.annotation.SuppressLint;
import android.app.AlertDialog.Builder;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.res.Resources;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.preference.EditTextPreference;
import android.text.InputType;
import android.util.AttributeSet;
import android.view.View;
import android.widget.EditText;

@SuppressLint("HandlerLeak")
public class CavanServicePreference extends EditTextPreference {

	private static final int EVENT_START_SERVICE = 1;
	private static final int EVENT_STOP_SERVICE = 2;
	private static final int EVENT_RESTART_SERVICE = 3;

	private static HashMap<String, Class<?>> mHashMap = new HashMap<String, Class<?>>();

	static {
		mHashMap.put("ftp_service", FtpService.class);
		mHashMap.put("tcp_dd_service", TcpDdService.class);
		mHashMap.put("web_proxy_service", WebProxyService.class);
	}

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			CavanUtils.logD("message = " + msg);

			switch (msg.what) {
			case EVENT_START_SERVICE:
				startService();
				break;

			case EVENT_STOP_SERVICE:
				stopService();
				break;

			case EVENT_RESTART_SERVICE:
				restartService();
				break;
			}

			super.handleMessage(msg);
		}
	};

	private boolean mNeedStop;
	private Class<?> mServiceClass;
	private ICavanService mService;
	private ServiceConnection mConnection = new ServiceConnection() {

		@Override
		public void onServiceDisconnected(ComponentName arg0) {
			CavanUtils.logD("onServiceDisconnected: " + arg0);

			try {
				getContext().unregisterReceiver(mReceiver);
			} catch (Exception e) {
				e.printStackTrace();
			}

			mService = null;

			updateSummary(false);
			setEnabled(false);

			mHandler.sendEmptyMessageDelayed(EVENT_START_SERVICE, 500);
		}

		@Override
		public void onServiceConnected(ComponentName arg0, IBinder arg1) {
			CavanUtils.logD("onServiceConnected: " + arg0);

			mService = ICavanService.Stub.asInterface(arg1);

			try {
				IntentFilter filter = new IntentFilter(mService.getAction());
				getContext().registerReceiver(mReceiver, filter);
			} catch (RemoteException e) {
				e.printStackTrace();
			}

			updateSummary(getState());
			setEnabled(true);
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
		startService(context);
	}

	public Intent getServiceIntent(Context context) {
		return new Intent(context, mServiceClass);
	}

	public void startService(Context context) {
		Intent service = getServiceIntent(context);
		context.startService(service);
		context.bindService(service, mConnection, 0);
	}

	public void startService() {
		startService(getContext());
	}

	public void stopService(Context context) {
		context.stopService(getServiceIntent(context));
	}

	public void stopService() {
		stopService(getContext());
	}

	public void restartService() {
		stopService();
		mHandler.sendEmptyMessageDelayed(EVENT_START_SERVICE, 500);
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

	public boolean stop() {
		if (mService != null) {
			try {
				return mService.stop();
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}

		return false;
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

	@Override
	protected void onDialogClosed(boolean positiveResult) {
		super.onDialogClosed(positiveResult);

		if (positiveResult) {
			if (mNeedStop) {
				stop();
			} else {
				start(Integer.parseInt(getText()));
			}
		}
	}

	@Override
	protected void showDialog(Bundle state) {
		mNeedStop = getState();
		setText(Integer.toString(getPort()));

		super.showDialog(state);
	}

	@Override
	protected void onAddEditTextToDialogView(View dialogView, EditText editText) {
		if (mNeedStop) {
			editText.setEnabled(false);
		} else {
			editText.setEnabled(true);
			editText.setInputType(InputType.TYPE_CLASS_NUMBER);
		}

		super.onAddEditTextToDialogView(dialogView, editText);
	}

	@Override
	protected void onPrepareDialogBuilder(Builder builder) {
		builder.setMessage(R.string.text_port);

		if (mNeedStop) {
			builder.setPositiveButton(R.string.text_stop, this);
		} else {
			builder.setPositiveButton(R.string.text_start, this);
		}

		super.onPrepareDialogBuilder(builder);
	}
}