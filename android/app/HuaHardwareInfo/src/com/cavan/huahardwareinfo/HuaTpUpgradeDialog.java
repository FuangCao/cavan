package com.cavan.huahardwareinfo;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import android.app.AlertDialog;
import android.content.Context;
import android.content.res.Resources;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.ServiceManager;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.view.WindowManager;
import android.widget.ProgressBar;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;

public class HuaTpUpgradeDialog extends AlertDialog {
	public static final int MSG_PROGRESS_CHANGED = 0;
	public static final int MSG_STATE_CHANGED = 1;
	public static final int MSG_SCAN_COMPLETE = 3;

	private static final String TAG = "Cavan";
	public static final int MAX_PROGRESS = 100;

	private View mView;
	private Toast mToast;
	private ProgressBar mProgressBar;
	private TextView mTextView;
	private RadioGroup mRadioGroup;
	private HuaHardwareInfoActivity mActivity;
	private String mFwName;
	private HuaTouchscreenDevice mTouchscreenDevice;
	private PowerManager mPowerManager;
	private WakeLock mWakeLock;
	private Handler mHandler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			Log.d(TAG, "msg.what = " + msg.what);

			switch (msg.what) {
			case MSG_PROGRESS_CHANGED:
				mTextView.setText(Integer.toString(msg.arg1) + "%");
				mProgressBar.setProgress(msg.arg1);
				break;

			case MSG_STATE_CHANGED:
				switch (msg.arg1) {
				case HuaTouchscreenDevice.FW_STATE_UPGRADE_FAILED:
					showToast(R.string.msg_fw_upgrade_faild, Toast.LENGTH_SHORT);
					dismiss();
					break;

				case HuaTouchscreenDevice.FW_STATE_UPGRADE_COMPLETE:
					showToast(R.string.msg_fw_upgrade_complete, Toast.LENGTH_SHORT);
					if (mActivity != null) {
						mActivity.loadTpInfo();
					}
					dismiss();
					break;

				case HuaTouchscreenDevice.FW_STATE_UPGRADE_PREPARE:
					mTextView.setText(R.string.msg_fw_upgrade_pepare);
					break;

				case HuaTouchscreenDevice.FW_STATE_UPGRADE_STOPPING:
					mTextView.setText(R.string.msg_fw_upgrade_stopping);
					break;
				}
				break;

			case MSG_SCAN_COMPLETE:
				List<File> files = (List<File>) msg.obj;
				if (files != null && files.size() > 0) {
					mRadioGroup.removeAllViews();
					for (File file : files) {
						FirmwareRadioButton button = new FirmwareRadioButton(getContext(), file);
						mRadioGroup.addView(button);
					}

					mRadioGroup.check(mRadioGroup.getChildAt(0).getId());

					if (mActivity == null) {
						upgradeFirmware();
					}
				} else {
					showToast(R.string.msg_fw_not_found, Toast.LENGTH_SHORT);
					dismiss();
				}
				break;
			}

			super.handleMessage(msg);
		}
	};

	protected HuaTpUpgradeDialog(HuaHardwareInfoActivity activity) {
		super(activity.getApplicationContext());

		mActivity = activity;
		mTouchscreenDevice = activity.getTouchscreenDevice();
		mFwName = mTouchscreenDevice.getFwName();
	}

	protected HuaTpUpgradeDialog(Context context, String fwName) {
		super(context);

		mActivity = null;
		mFwName = fwName;
		mTouchscreenDevice = HuaTouchscreenDevice.getTouchscreenDevice();
	}

	private void showToast(String message, int duration) {
		if (mToast != null) {
			mToast.cancel();
		}

		Log.d(TAG, message);

		mToast = Toast.makeText(getContext(), message, duration);
		mToast.setGravity(Gravity.BOTTOM, 0, 0);
		mToast.show();
	}

	private void showToast(int resId, int duration) {
		showToast(getContext().getResources().getString(resId), duration);
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		mPowerManager = (PowerManager) getContext().getSystemService(Context.POWER_SERVICE);
		mWakeLock = mPowerManager.newWakeLock(PowerManager.ACQUIRE_CAUSES_WAKEUP | PowerManager.SCREEN_BRIGHT_WAKE_LOCK, getClass().getName());
		mWakeLock.acquire();

		mView = getLayoutInflater().inflate(R.layout.tp_upgrade_progress, null);
		setView(mView);
		getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);

		mProgressBar = (ProgressBar) mView.findViewById(R.id.progressBar);
		mProgressBar.setMax(MAX_PROGRESS);
		mProgressBar.setProgress(0);

		mTextView = (TextView) mView.findViewById(R.id.textView);
		mRadioGroup = (RadioGroup) mView.findViewById(R.id.radioGroup);

		Resources resources = getContext().getResources();
		setTitle(resources.getString(R.string.info_fw_upgrade));

		setButton(BUTTON_POSITIVE, resources.getString(R.string.start_upgrade), mHandler.obtainMessage(BUTTON_POSITIVE));
		setButton(BUTTON_NEGATIVE, resources.getString(android.R.string.cancel), mHandler.obtainMessage(BUTTON_NEGATIVE));

		super.onCreate(savedInstanceState);

		getButton(BUTTON_POSITIVE).setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				upgradeFirmware();
			}
		});

		scanFirmware();
	}

	@Override
	public void dismiss() {
		mWakeLock.release();
		super.dismiss();
	}

	private void upgradeFirmware() {
		int id = mRadioGroup.getCheckedRadioButtonId();
		Log.d(TAG, "index = " + id + ", count = " + mRadioGroup.getChildCount());
		if (id < 0) {
			showToast(R.string.msg_select_fw, Toast.LENGTH_SHORT);
		} else {
			showToast(R.string.msg_tp_fw_upgrade_waring, Toast.LENGTH_LONG);
			FirmwareRadioButton button = (FirmwareRadioButton) mRadioGroup.findViewById(id);
			File file = button.getFileFirmware();
			Log.d(TAG, "firmware path = " + file.getPath());
			mTouchscreenDevice.setFileFw(file);
			setCancelable(false);
			mRadioGroup.setEnabled(false);
			getButton(BUTTON_POSITIVE).setEnabled(false);
			getButton(BUTTON_NEGATIVE).setEnabled(false);
			mTextView.setText(R.string.msg_fw_upgrade_pepare);
			mTouchscreenDevice.fwUpgrade(MAX_PROGRESS, mHandler);
		}
	}

	private void scanFirmware(List<File> list, File dir, int depth) {
		File fileFw = new File(dir, mFwName);
		if (fileFw.canRead()) {
			list.add(fileFw);
		}

		if (depth < 2) {
			return;
		}

		File[] files = dir.listFiles();
		if (files == null) {
			return;
		}

		for (File file : files) {
			if (file.isDirectory() && file.getName().startsWith(".") == false) {
				scanFirmware(list, file, depth - 1);
			}
		}
	}

	private void scanFirmware() {
		List<File> list = new ArrayList<File>();

		File dir = Environment.getDataDirectory();
		scanFirmware(list, dir, 1);
		scanFirmware(list, new File(dir, "internal_memory"), 1);

		dir = Environment.getExternalStorageDirectory();
		scanFirmware(list, dir, 1);
		scanFirmware(list, new File(dir, "tp/firmware"), 2);

		Message message = mHandler.obtainMessage(MSG_SCAN_COMPLETE, list);
		message.sendToTarget();
	}

	class FirmwareRadioButton extends RadioButton {
		private File mFileFirmware;

		public FirmwareRadioButton(Context context, File fileFirmware) {
			super(context);
			setFileFirmware(fileFirmware);
		}

		public void setFileFirmware(File fileFirmware) {
			mFileFirmware = fileFirmware;
			setText(mFileFirmware.getPath());
		}

		public File getFileFirmware() {
			return mFileFirmware;
		}
	}
}
