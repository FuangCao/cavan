package com.cavan.huahardwareinfo;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import android.app.AlertDialog;
import android.content.res.Resources;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.BaseAdapter;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.CompoundButton;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.RadioButton;
import android.widget.TextView;
import android.widget.Toast;

public class HuaTpUpgradeDialog extends AlertDialog {
	public static final int MSG_PROGRESS_CHANGED = 0;
	public static final int MSG_STATE_CHANGED = 1;
	public static final int MSG_SCAN_COMPLETE = 3;

	private static final String TAG = "Cavan";
	public static final int MAX_PROGRESS = 100;

	private View mView;
	private ProgressBar mProgressBar;
	private TextView mTextView;
	private ListView mListView;
	private HuaHardwareInfoActivity mActivity;
	private HuaTouchscreenDevice mTouchscreenDevice;
	private List<File> mListFw = new ArrayList<File>();
	private File mFileSelected;
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
			case MSG_SCAN_COMPLETE:
				break;
			}

			super.handleMessage(msg);
		}
	};

	private ListAdapter mAdapter = new BaseAdapter() {
		private CompoundButton mButtonSelected;
		private OnCheckedChangeListener mListener = new OnCheckedChangeListener() {
			@Override
			public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
				if (mButtonSelected != null) {
					mButtonSelected.setChecked(false);
				}

				mButtonSelected = buttonView;
				mFileSelected = mListFw.get(mButtonSelected.getId());
			}
		};

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			RadioButton button = new RadioButton(getContext());
			button.setText(mListFw.get(position).getPath());
			button.setOnCheckedChangeListener(mListener);
			button.setId(position);
			return button;
		}

		@Override
		public long getItemId(int position) {
			return 0;
		}

		@Override
		public Object getItem(int position) {
			return null;
		}

		@Override
		public int getCount() {
			return mListFw.size();
		}
	};

	protected HuaTpUpgradeDialog(HuaHardwareInfoActivity activity) {
		super(activity.getApplicationContext());

		mActivity = activity;
		mTouchscreenDevice = activity.getTouchscreenDevice();
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		mView = getLayoutInflater().inflate(R.layout.tp_upgrade_progress, null);
		setView(mView);
		getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);

		mProgressBar = (ProgressBar) mView.findViewById(R.id.progressBar);
		mProgressBar.setMax(MAX_PROGRESS);
		mProgressBar.setProgress(0);

		mTextView = (TextView) mView.findViewById(R.id.textView);

		mListView = (ListView) mView.findViewById(R.id.listView);
		mListView.setChoiceMode(ListView.CHOICE_MODE_SINGLE);
		mListView.setAdapter(mAdapter);

		Resources resources = getContext().getResources();
		setTitle(resources.getString(R.string.info_fw_upgrade));

		setButton(BUTTON_POSITIVE, resources.getString(R.string.start_upgrade), mHandler.obtainMessage(BUTTON_POSITIVE));
		setButton(BUTTON_NEGATIVE, resources.getString(android.R.string.cancel), mHandler.obtainMessage(BUTTON_NEGATIVE));

		super.onCreate(savedInstanceState);

		getButton(BUTTON_POSITIVE).setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				if (mFileSelected == null) {
					Toast.makeText(getContext(), getContext().getResources().getString(R.string.msg_select_fw), Toast.LENGTH_SHORT).show();
				} else {
					mTouchscreenDevice.setFileFw(mFileSelected);
					setCancelable(false);
					mListView.setEnabled(false);
					getButton(BUTTON_POSITIVE).setEnabled(false);
					getButton(BUTTON_NEGATIVE).setEnabled(false);
					mTextView.setText(R.string.msg_fw_upgrade_pepare);
					mTouchscreenDevice.fwUpgrade(MAX_PROGRESS, mHandler);
					Toast.makeText(getContext(), getContext().getString(R.string.msg_tp_fw_upgrade_waring), Toast.LENGTH_SHORT).show();
				}
			}
		});

		scanFirmware();
	}

	private void scanFirmware(File dir, String filename) {
		File[] files = dir.listFiles();
		if (files == null) {
			return;
		}

		for (File file : files) {
			if (file.isDirectory()) {
				if (file.getName().startsWith(".") == false) {
					scanFirmware(file, filename);
				}
			} else if (filename.equals(file.getName())) {
				mListFw.add(file);
			}
		}
	}

	private void scanFirmware() {
		mListFw.add(mTouchscreenDevice.getFileFw());
		scanFirmware(Environment.getExternalStorageDirectory(), mTouchscreenDevice.getFwName());
		mHandler.sendEmptyMessage(MSG_SCAN_COMPLETE);
	}
}
