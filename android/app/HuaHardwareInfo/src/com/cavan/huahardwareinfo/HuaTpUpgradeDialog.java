package com.cavan.huahardwareinfo;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import android.app.ActivityManager;
import android.app.AlertDialog;
import android.app.KeyguardManager;
import android.app.KeyguardManager.KeyguardLock;
import android.content.Context;
import android.content.DialogInterface;
import android.content.res.Resources;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.view.WindowManager;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.ProgressBar;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;

import com.cavan.huahardwareinfo.R;

public class HuaTpUpgradeDialog extends AlertDialog implements OnCheckedChangeListener {
	public static final int MSG_PROGRESS_CHANGED = 0;
	public static final int MSG_STATE_CHANGED = 1;
	public static final int MSG_SCAN_COMPLETE = 3;
	public static final int MSG_DISMISS = 4;
	public static final int MSG_FW_RECOVERY = 5;

	private static final String TAG = "Cavan";
	public static final int MAX_PROGRESS = 100;
	private File mDirScanAuto = new File("/system/firmware");
	private File[] mDirScanList = {
		new File("/system"),
		new File("/system/etc"),
		Environment.getDataDirectory(),
		new File(Environment.getDataDirectory(), "internal_memory"),
		Environment.getExternalStorageDirectory(),
		new File("/storage/sdcard1")
	};

	private AlertDialog mDialog;
	private View mView;
	private Toast mToast;
	private ProgressBar mProgressBar;
	private TextView mTextView;
	private RadioGroup mRadioGroup;
	private CheckBox mCheckBoxAutoUpgrade;
	private HuaHardwareInfoActivity mActivity;
	private String mFwName;
	private boolean mAutoUpgrade;
	private boolean mAutoRecovery;
	private HuaTouchscreenDevice mTouchscreenDevice;
	private PowerManager mPowerManager;
	private WakeLock mWakeLock;
	private KeyguardLock mKeyguardLock;
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
					showWarningDialog(true, 20);
					break;

				case HuaTouchscreenDevice.FW_STATE_UPGRADE_COMPLETE:
					mTouchscreenDevice.fillVendorInfo();
					String newName = mTouchscreenDevice.getFwName();
					String oldName = HuaTouchscreenDevice.getPendingFirmware(getContext());
					Log.d(TAG, "newName = " + newName + ", oldName = " + oldName);
					if (mActivity != null && (newName == null || newName.equals(oldName) == false)) {
						showWarningDialog(false, 30);
					} else {
						HuaTouchscreenDevice.setPendingFirmware(getContext(), "");
						showToast(R.string.msg_fw_upgrade_complete, Toast.LENGTH_SHORT, true);
						if (mActivity != null) {
							mActivity.loadTpInfo();
						}
						mHandler.sendEmptyMessageDelayed(MSG_DISMISS, 1000);
					}
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
					if (mAutoRecovery) {
						upgradeFirmware();
					} else if (mAutoUpgrade) {
						showWarningDialog(false, 30);
					}
				} else {
					showToast(R.string.msg_fw_not_found, Toast.LENGTH_SHORT, true);
					mHandler.sendEmptyMessageDelayed(MSG_DISMISS, 1000);
				}
				break;

			case MSG_DISMISS:
				dismiss();
				break;

			case MSG_FW_RECOVERY:
				if (msg.arg1 > 0) {
					sendRecoveryMessage(msg.arg1 - 1);
				} else {
					recoveryFirmware();
				}
				break;
			}

			super.handleMessage(msg);
		}
	};

	private OnClickListener mClickListener = new OnClickListener() {
		@Override
		public void onClick(DialogInterface dialog, int which) {
			mHandler.removeMessages(MSG_FW_RECOVERY);

			if (mToast != null) {
				mToast.cancel();
			}

			switch (which) {
			case DialogInterface.BUTTON_POSITIVE:
				recoveryFirmware();
				break;

			case DialogInterface.BUTTON_NEGATIVE:
				HuaTouchscreenDevice.setPendingFirmware(getContext(), "");
				break;
			}

			dialog.dismiss();
		}
	};

	protected HuaTpUpgradeDialog(HuaHardwareInfoActivity activity) {
		super(activity.getApplicationContext());

		mActivity = activity;
		mTouchscreenDevice = activity.getTouchscreenDevice();
		mFwName = mTouchscreenDevice.getFwName();
	}

	protected HuaTpUpgradeDialog(Context context, HuaTouchscreenDevice touchscreenDevice, String fwName) {
		super(context);

		mActivity = null;
		mTouchscreenDevice = touchscreenDevice;
		if (fwName == null) {
			mAutoUpgrade = true;
			mFwName = mTouchscreenDevice.getFwName();
		} else {
			mAutoRecovery = true;
			mFwName = fwName;
		}

		setCancelable(false);
	}

	private void showToast(String message, int duration, boolean updateTextView) {
		if (mToast != null) {
			mToast.cancel();
		}

		Log.d(TAG, message);

		if (updateTextView) {
			mTextView.setText(message);
		}

		mToast = Toast.makeText(getContext(), message, duration);
		mToast.setGravity(Gravity.BOTTOM, 0, 0);
		mToast.show();
	}

	private void showToast(int resId, int duration, boolean updateTextView) {
		showToast(getContext().getResources().getString(resId), duration, updateTextView);
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		Log.d(TAG, "mAutoUpgrade = " + mAutoUpgrade);
		Log.d(TAG, "mAutoRecovery = " + mAutoRecovery);

		mPowerManager = (PowerManager) getContext().getSystemService(Context.POWER_SERVICE);
		mWakeLock = mPowerManager.newWakeLock(PowerManager.ACQUIRE_CAUSES_WAKEUP | PowerManager.SCREEN_BRIGHT_WAKE_LOCK, getClass().getName());
		mWakeLock.acquire();

		if (mAutoRecovery || mAutoUpgrade) {
			KeyguardManager keyguardManager  = (KeyguardManager) getContext().getSystemService(Context.KEYGUARD_SERVICE);
			mKeyguardLock = keyguardManager.newKeyguardLock(getClass().getName());
			if (mKeyguardLock != null) {
				mKeyguardLock.disableKeyguard();
			}
		}

		mView = getLayoutInflater().inflate(R.layout.tp_upgrade_progress, null);
		setView(mView);
		getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);

		mProgressBar = (ProgressBar) mView.findViewById(R.id.progressBar);
		mProgressBar.setMax(MAX_PROGRESS);
		mProgressBar.setProgress(0);

		mTextView = (TextView) mView.findViewById(R.id.textView);
		mRadioGroup = (RadioGroup) mView.findViewById(R.id.radioGroup);

		mCheckBoxAutoUpgrade = (CheckBox) mView.findViewById(R.id.checkBoxAutoUpgrade);
		mCheckBoxAutoUpgrade.setChecked(HuaTouchscreenDevice.getAutoUpgrade(getContext()));
		mCheckBoxAutoUpgrade.setOnCheckedChangeListener(this);

		Resources resources = getContext().getResources();
		setTitle(resources.getString(R.string.info_fw_upgrade));

		setButton(BUTTON_POSITIVE, resources.getString(R.string.start_upgrade), mHandler.obtainMessage(BUTTON_POSITIVE));
		setButton(BUTTON_NEGATIVE, resources.getString(android.R.string.cancel), mHandler.obtainMessage(BUTTON_NEGATIVE));

		super.onCreate(savedInstanceState);

		if (ActivityManager.isUserAMonkey()) {
    		Log.d(TAG, getClass() + "in monkey test mode");
    	} else {
	    	getButton(BUTTON_POSITIVE).setOnClickListener(new View.OnClickListener() {
				@Override
				public void onClick(View v) {
					upgradeFirmware();
				}
			});

    		scanFirmware();
    	}
	}

	@Override
	public void dismiss() {
		Log.d(TAG, getClass() + " dismiss()");

		mHandler.removeMessages(MSG_FW_RECOVERY);

		if (mWakeLock.isHeld()) {
			mWakeLock.release();
		}

		if (mKeyguardLock != null) {
			mKeyguardLock.reenableKeyguard();
		}

		super.dismiss();
	}

	private void upgradeFirmware() {
		int id = mRadioGroup.getCheckedRadioButtonId();
		Log.d(TAG, "index = " + id + ", count = " + mRadioGroup.getChildCount());
		if (id < 0) {
			showToast(R.string.msg_select_fw, Toast.LENGTH_SHORT, false);
		} else {
			showToast(R.string.msg_tp_fw_upgrade_waring, Toast.LENGTH_LONG, false);
			FirmwareRadioButton button = (FirmwareRadioButton) mRadioGroup.findViewById(id);
			File file = button.getFileFirmware();
			Log.d(TAG, "firmware path = " + file.getPath());
			mTouchscreenDevice.setFileFw(file);
			setCancelable(false);
			mRadioGroup.setEnabled(false);
			getButton(BUTTON_POSITIVE).setEnabled(false);
			getButton(BUTTON_NEGATIVE).setEnabled(false);
			mTextView.setText(R.string.msg_fw_upgrade_pepare);
			mTouchscreenDevice.fwUpgrade(getContext(), MAX_PROGRESS, mHandler);
		}
	}

	private boolean recoveryFirmware() {
		View view = mRadioGroup.getChildAt(0);
		if (view == null) {
			return false;
		}

		if (mDialog != null) {
			mDialog.dismiss();
		}

		show();
		mRadioGroup.check(view.getId());
		upgradeFirmware();

		return true;
	}

	private void scanFirmware(List<File> list, File dir, int depth) {
		Log.d(TAG, "scan firmware from " + dir.getPath());

		File fileFw = new File(dir, mFwName);
		if (fileFw.canRead() && list.contains(fileFw) == false) {
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

		if (mAutoUpgrade) {
			scanFirmware(list, mDirScanAuto, 1);
		} else {
			for (File dir : mDirScanList) {
				scanFirmware(list, dir, 1);
				scanFirmware(list, new File(dir, "firmware"), 2);
				scanFirmware(list, new File(dir, "tp/firmware"), 2);
			}
		}

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

	private void sendRecoveryMessage(int delaySecond) {
		int id = mAutoUpgrade ? R.string.msg_auto_upgrade : R.string.msg_auto_recovery;
		String text = getContext().getResources().getString(id, delaySecond);
		Log.d(TAG, text);
		if (text != null) {
			if (mDialog != null) {
				mDialog.setMessage(text);
				if (mDialog.isShowing() == false) {
					mDialog.show();
				}
			} else {
				mTextView.setText(text);
			}
		}

		Message message = mHandler.obtainMessage(MSG_FW_RECOVERY);
		message.arg1 = delaySecond;
		mHandler.sendMessageDelayed(message, 1000);
	}

	private boolean showWarningDialog(boolean isFailed, int delaySecond)
	{
		if (mToast != null) {
			mToast.cancel();
		}

		if (mDialog == null && mActivity != null) {
			AlertDialog.Builder builder = new AlertDialog.Builder(mActivity);

			if (mAutoUpgrade) {
				builder.setNegativeButton(R.string.donot_upgrade, mClickListener);
				builder.setPositiveButton(R.string.upgrade_immediate, mClickListener);
			} else {
				builder.setNegativeButton(R.string.msg_donot_recovery, mClickListener);
				builder.setPositiveButton(R.string.msg_recovery_immediate, mClickListener);
			}

			builder.setCancelable(false);
			mDialog = builder.create();
		}

		if (mDialog != null) {
			if (isFailed) {
				mDialog.setTitle(R.string.msg_fw_upgrade_faild);
			} else if (mAutoUpgrade) {
				mDialog.setTitle(R.string.msg_new_fw_found);
			} else {
				mDialog.setTitle(R.string.msg_fw_not_match);
			}

			dismiss();
		} else {
			getButton(BUTTON_NEGATIVE).setEnabled(true);
		}

		sendRecoveryMessage(delaySecond);

		return true;
	}

	@Override
	public void onCheckedChanged(CompoundButton arg0, boolean arg1) {
		Log.d(TAG, "onCheckedChanged");
		HuaTouchscreenDevice.setAutoUpgrade(getContext(), arg1);
	}
}
