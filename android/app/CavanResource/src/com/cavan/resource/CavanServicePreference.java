package com.cavan.resource;

import android.annotation.SuppressLint;
import android.app.AlertDialog.Builder;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
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

import com.cavan.android.CavanAndroid;
import com.cavan.service.CavanServiceState;
import com.cavan.service.ICavanService;
import com.cavan.service.ICavanServiceCallback;

@SuppressLint("HandlerLeak")
public abstract class CavanServicePreference extends EditTextPreference {

	private static final int EVENT_START_SERVICE = 1;
	private static final int EVENT_STOP_SERVICE = 2;
	private static final int EVENT_RESTART_SERVICE = 3;
	private static final int EVENT_STATE_CHANGED = 4;

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
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

			case EVENT_STATE_CHANGED:
				updateSummary(msg.arg1);
				break;
			}
		}
	};

	private boolean mNeedStop;
	private ICavanService mService;

	private ICavanServiceCallback mCallback = new ICavanServiceCallback.Stub() {

		@Override
		public void onServiceStateChanged(int state) throws RemoteException {
			Message message = mHandler.obtainMessage(EVENT_STATE_CHANGED);
			message.arg1 = state;
			message.sendToTarget();
		}
	};

	private ServiceConnection mConnection = new ServiceConnection() {

		@Override
		public void onServiceDisconnected(ComponentName arg0) {
			CavanAndroid.dLog("onServiceDisconnected: " + arg0);

			mService = null;

			updateSummary(CavanServiceState.STOPPED);
			setEnabled(false);

			mHandler.sendEmptyMessageDelayed(EVENT_START_SERVICE, 500);
		}

		@Override
		public void onServiceConnected(ComponentName arg0, IBinder arg1) {
			CavanAndroid.dLog("onServiceConnected: " + arg0);

			mService = ICavanService.Stub.asInterface(arg1);

			try {
				mService.addCallback(mCallback);
			} catch (RemoteException e) {
				e.printStackTrace();
			}

			setEnabled(true);
		}
	};

	public abstract Intent getServiceIntent(Context context);

	public CavanServicePreference(Context context, AttributeSet attrs) {
		super(context, attrs);

		updateSummary(CavanServiceState.STOPPED);
		startService(context);
	}

	public synchronized void unbindService(Context context) {
		if (mService != null) {
			try {
				mService.removeCallback(mCallback);
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}

		context.unbindService(mConnection);
	}

	public synchronized void startService(Context context) {
		Intent service = getServiceIntent(context);
		context.startService(service);
		context.bindService(service, mConnection, 0);
	}

	public synchronized void startService() {
		startService(getContext());
	}

	public synchronized void stopService(Context context) {
		context.stopService(getServiceIntent(context));
	}

	public synchronized void stopService() {
		stopService(getContext());
	}

	public synchronized void restartService() {
		stopService();
		mHandler.sendEmptyMessageDelayed(EVENT_START_SERVICE, 500);
	}

	public synchronized int getServiceState() {
		if (mService != null) {
			try {
				return mService.getState();
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}

		return CavanServiceState.STOPPED;
	}

	public synchronized boolean isServiceEnabled() {
		if (mService != null) {
			try {
				return mService.isEnabled();
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}

		return false;
	}

	public synchronized boolean start(int port) {
		if (mService != null) {
			try {
				mService.start(port);
				return true;
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}

		return false;
	}

	public synchronized boolean start() {
		String text = getText();
		if (text != null && text.length() > 0) {
			try {
				return start(Integer.parseInt(text));
			} catch (NumberFormatException e) {
				e.printStackTrace();
			}
		}

		return false;
	}

	public synchronized boolean checkAndStart() {
		if (isServiceEnabled()) {
			return true;
		}

		return start();
	}

	public synchronized void stop() {
		if (mService != null) {
			try {
				mService.stop();
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}
	}

	public synchronized int getPort() {
		if (mService != null) {
			try {
				return mService.getPort();
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}

		return 0;
	}

	public synchronized void updateSummary(int state) {

		if (mService == null) {
			setSummary(R.string.service_disconnected);
		} else {
			Resources resources = getContext().getResources();
			if (resources == null) {
				setSummary(R.string.service_connected);
			} else {
				StringBuilder builder = new StringBuilder();

				int resId;

				switch (state) {
				case CavanServiceState.PREPARE:
					resId = R.string.prepare;
					break;

				case CavanServiceState.RUNNING:
					resId = R.string.running;
					break;

				case CavanServiceState.WAITING:
					resId = R.string.waiting;
					break;

				case CavanServiceState.STOPPED:
					resId = R.string.stopped;
					break;

				default:
					resId = R.string.unknown;
				}

				builder.append(resources.getString(resId));
				builder.append(", ");
				builder.append(resources.getString(R.string.port));
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
				start();
			}
		}
	}

	@Override
	protected void showDialog(Bundle state) {
		mNeedStop = isServiceEnabled();
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
		builder.setMessage(R.string.port);

		if (mNeedStop) {
			builder.setPositiveButton(R.string.stop, this);
		} else {
			builder.setPositiveButton(R.string.start, this);
		}

		super.onPrepareDialogBuilder(builder);
	}
}
