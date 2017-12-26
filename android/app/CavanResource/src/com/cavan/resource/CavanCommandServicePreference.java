package com.cavan.resource;

import android.app.AlertDialog.Builder;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.preference.EditTextPreference;
import android.util.AttributeSet;
import android.view.View;
import android.widget.EditText;

import com.cavan.android.CavanAndroid;
import com.cavan.service.CavanCommandService;
import com.cavan.service.CavanServiceState;
import com.cavan.service.ICavanCommandService;
import com.cavan.service.ICavanCommandServiceCallback;

public class CavanCommandServicePreference extends EditTextPreference {

	private static final int MSG_STATE_CHANGED = 1;

	private ICavanCommandService mService;
	private boolean mNeedStop;

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case MSG_STATE_CHANGED:
				updateSummary(msg.arg1);
				break;
			}
		}
	};

	private ICavanCommandServiceCallback mCallback = new ICavanCommandServiceCallback.Stub() {

		@Override
		public void onServiceStateChanged(int state) throws RemoteException {
			Message message = mHandler.obtainMessage(MSG_STATE_CHANGED);
			message.arg1 = state;
			message.sendToTarget();
		}
	};

	private ServiceConnection mConnection = new ServiceConnection() {

		@Override
		public void onServiceDisconnected(ComponentName name) {
			mService = null;
			setEnabled(false);
		}

		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			mService = ICavanCommandService.Stub.asInterface(service);

			try {
				mService.addCallback(mCallback);
			} catch (RemoteException e) {
				e.printStackTrace();
			}

			setEnabled(true);
		}
	};

	public CavanCommandServicePreference(Context context) {
		super(context);
		startService();
	}

	public CavanCommandServicePreference(Context context, AttributeSet attrs) {
		super(context, attrs);
		startService();
	}

	public CavanCommandServicePreference(Context context, AttributeSet attrs, int defStyleAttr) {
		super(context, attrs, defStyleAttr);
		startService();
	}

	public CavanCommandServicePreference(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
		super(context, attrs, defStyleAttr, defStyleRes);
		startService();
	}

	public Intent getServiceIntent(Context context) {
		return new Intent(context, CavanCommandService.class);
	}

	public void startService() {
		setEnabled(false);
		Context context = getContext();
		Intent intent = getServiceIntent(context);
		CavanAndroid.startAndBindService(context, intent, mConnection);
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

	public synchronized void start() {
		if (mService != null) {
			String command = getText();

			try {
				mService.start(command);
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}
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

	public String getCommand() {
		if (mService != null) {
			try {
				return mService.getCommand();
			} catch (RemoteException e) {
				e.printStackTrace();
			};
		}

		return null;
	}

	public void updateSummary(int state) {
		int summary;

		switch (state) {
		case CavanServiceState.RUNNING:
			summary = R.string.running;
			break;

		case CavanServiceState.STOPPED:
			summary = R.string.stopped;
			break;

		case CavanServiceState.WAITING:
			summary = R.string.waiting;
			break;

		default:
			summary = R.string.unknown;
		}

		setSummary(summary);
	}

	public synchronized void unbindService(Context context) {
		if (mService != null) {
			try {
				mService.removeCallback(mCallback);
			} catch (RemoteException e) {
				e.printStackTrace();
			}

			context.unbindService(mConnection);
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

		String command = getCommand();
		if (command != null) {
			setText(command);
		}

		super.showDialog(state);
	}

	@Override
	protected void onAddEditTextToDialogView(View dialogView, EditText editText) {
		if (mNeedStop) {
			editText.setEnabled(false);
		} else {
			editText.setEnabled(true);
		}

		super.onAddEditTextToDialogView(dialogView, editText);
	}

	@Override
	protected void onPrepareDialogBuilder(Builder builder) {
		builder.setMessage(R.string.cmdline);

		if (mNeedStop) {
			builder.setPositiveButton(R.string.stop, this);
		} else {
			builder.setPositiveButton(R.string.start, this);
		}

		super.onPrepareDialogBuilder(builder);
	}
}
