package com.cavan.huahardwareinfo;

import java.io.File;

import android.app.AlertDialog;
import android.content.res.Resources;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

public class HuaTpUpgradeDialog extends AlertDialog {
	public static final int MSG_PROGRESS_CHANGED = 0;
	public static final int MSG_STATE_CHANGED = 1;

	private static final String TAG = "Cavan";
	public static final int MAX_PROGRESS = 100;

	private View mView;
	private ProgressBar mProgressBar;
	private TextView mTextView;
	private HuaHardwareInfoActivity mActivity;
	private HuaTouchscreenDevice mTouchscreenDevice;
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
				int resId;
				if (msg.arg1 < 0) {
					resId = R.string.msg_fw_upgrade_faild;
				} else {
					resId = R.string.msg_fw_upgrade_complete;
					mActivity.loadTpInfo();
				}

				Toast.makeText(getContext(), getContext().getString(resId), Toast.LENGTH_SHORT).show();
				dismiss();
				break;
			}
			super.handleMessage(msg);
		}

	};

	protected HuaTpUpgradeDialog(HuaHardwareInfoActivity activity) {
		super(activity.getApplicationContext());

		mActivity = activity;
		mTouchscreenDevice = activity.getTouchscreenDevice();
	}

	@Override
	protected void finalize() throws Throwable {
		super.finalize();
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		mView = getLayoutInflater().inflate(R.layout.tp_upgrade_progress, null);
		setView(mView);
		setInverseBackgroundForced(true);
		getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);

		mProgressBar = (ProgressBar) mView.findViewById(R.id.progressBar);
		mProgressBar.setMax(MAX_PROGRESS);
		mProgressBar.setProgress(0);

		mTextView = (TextView) mView.findViewById(R.id.textView);

		Resources resources = getContext().getResources();
		setTitle(resources.getString(R.string.info_fw_upgrade));

		setButton(BUTTON_POSITIVE, resources.getString(R.string.start_upgrade), mHandler.obtainMessage(BUTTON_POSITIVE));
		setButton(BUTTON_NEGATIVE, resources.getString(android.R.string.cancel), mHandler.obtainMessage(BUTTON_NEGATIVE));

		super.onCreate(savedInstanceState);

		getButton(BUTTON_POSITIVE).setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				setCancelable(false);
				getButton(BUTTON_POSITIVE).setEnabled(false);
				getButton(BUTTON_NEGATIVE).setEnabled(false);
				mTextView.setText(R.string.msg_fw_upgrade_pepare);
				mTouchscreenDevice.fwUpgrade(MAX_PROGRESS, mHandler);
			}
		});

		Toast.makeText(getContext(), getContext().getString(R.string.msg_tp_fw_upgrade_waring), Toast.LENGTH_LONG).show();
	}
}
