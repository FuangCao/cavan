package com.cavan.apkbackup;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import android.Manifest;
import android.app.Activity;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanCheckBox;
import com.cavan.cavanmain.R;
import com.cavan.java.CavanFile;

public class MainActivity extends Activity implements OnClickListener, OnCheckedChangeListener, TextWatcher {

	private static final CavanFile sOutDir = new CavanFile(Environment.getExternalStorageDirectory(), "cavan-apk-backup");

	private static final int MSG_BACKUP_START = 1;
	private static final int MSG_BACKUP_END = 2;
	private static final int MSG_COPY_START = 3;
	private static final int MSG_COPY_END = 4;
	private static final int MSG_CLEAR = 5;

	private static final String[] PERMISSIONS = {
		Manifest.permission.WRITE_EXTERNAL_STORAGE,
	};

	private ProgressBar mProgressBar;
	private TextView mTextViewState;
	private Button mButtonStart;
	private Button mButtonStop;
	private ListView mListViewApps;
	private EditText mEditTextSearch;
	private CheckBox mCheckBoxBackupSysApp;
	private CheckBox mCheckBoxClearBeforeBackup;
	private CavanCheckBox mCheckBoxSelectAll;

	private PackageManager mPackageManager;
	private BackupThread mThread;
	private LocalAdapter mAdapter = new LocalAdapter();
	private List<LocalPackageInfo> mPackageInfos = new ArrayList<LocalPackageInfo>();

	class BackupThread extends Thread {

		private boolean mNeedStop;

		public void setNeedStop() {
			mNeedStop = true;
		}

		private boolean doBackupFile(File inFile, CavanFile outFile) {
			if (inFile.length() == outFile.length() && outFile.isNewOrEqualThen(inFile)) {
				return true;
			}

			CavanAndroid.dLog("backup: " + inFile.getPath() + " => " + outFile.getPath());

			return outFile.copyFrom(inFile);
		}

		@Override
		public void run() {
			if (sOutDir.mkdirSafe()) {
				mHandler.sendEmptyMessage(MSG_BACKUP_START);

				if (mCheckBoxClearBeforeBackup.isChecked()) {
					Message message = mHandler.obtainMessage(MSG_CLEAR, sOutDir.getPath());
					message.sendToTarget();
					sOutDir.clear();
				}

				int progress = 0;

				for (LocalPackageInfo info : mPackageInfos) {
					if (mNeedStop) {
						break;
					}

					if (info.isEnabled()) {
						CavanFile outFile = new CavanFile(sOutDir, info.getBackupName());

						Message message = mHandler.obtainMessage(MSG_COPY_START, outFile.getPath());
						message.sendToTarget();

						if (!doBackupFile(info.getSourceFile(), outFile)) {
							break;
						}
					}

					Message message = mHandler.obtainMessage(MSG_COPY_END, ++progress, 0);
					message.sendToTarget();
				}

				mHandler.sendEmptyMessage(MSG_BACKUP_END);
			}

			mThread = null;
		}
	}

	public class LocalPackageInfo implements OnCheckedChangeListener {

		private String mName;
		private boolean mEnable;
		private PackageInfo mPackageInfo;

		public LocalPackageInfo(PackageInfo info) {
			mPackageInfo = info;
			mEnable = mCheckBoxSelectAll.isChecked();

			CharSequence label = mPackageManager.getApplicationLabel(info.applicationInfo);
			if (label == null) {
				mName = info.packageName;
			} else {
				mName = label.toString();
			}
		}

		public PackageInfo getPackageInfo() {
			return mPackageInfo;
		}

		public ApplicationInfo getApplicationInfo() {
			return mPackageInfo.applicationInfo;
		}

		public void setEnable(boolean enable) {
			mEnable = enable;
		}

		public boolean isEnabled() {
			return mEnable;
		}

		public String getApplicationName() {
			return mName;
		}

		public String getPackageName() {
			return mPackageInfo.packageName;
		}

		public int getVersionCode() {
			return mPackageInfo.versionCode;
		}

		public String getVersionName() {
			return mPackageInfo.versionName;
		}

		public String getSourcePath() {
			return getApplicationInfo().sourceDir;
		}

		public File getSourceFile() {
			return new File(getSourcePath());
		}

		public String getBackupName() {
			StringBuilder builder = new StringBuilder();

			builder.append(CavanFile.replaceInvalidFilenameChar(mName, '_'));
			builder.append('-');
			builder.append(getVersionName());
			builder.append(".apk");

			return builder.toString();
		}

		public int getFlags() {
			return getApplicationInfo().flags;
		}

		public boolean isSystemApp() {
			return (getFlags() & ApplicationInfo.FLAG_SYSTEM) != 0;
		}

		public void setupView(CavanCheckBox view, int position) {
			view.setText(position + ". " + mName);

			if (isSystemApp()) {
				view.setTextColor(Color.BLUE);
			} else {
				view.setTextColor(Color.BLACK);
			}

			view.setCheckedSilent(mEnable);
			view.setOnCheckedChangeListener(this);
		}

		public boolean isNeedBackup(String filter) {
			if (!mCheckBoxBackupSysApp.isChecked() && isSystemApp()) {
				return false;
			}

			if (filter.isEmpty()) {
				return true;
			}

			return mName.contains(filter);
		}

		@Override
		public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
			mEnable = isChecked;

			if (isChecked) {
				isChecked = isAllEnabled();
			}

			mCheckBoxSelectAll.setCheckedSilent(isChecked);
		}
	}

	public class LocalAdapter extends BaseAdapter {

		public void updateData() {
			mPackageInfos.clear();

			String filter = mEditTextSearch.getText().toString();

			for (PackageInfo info : mPackageManager.getInstalledPackages(0)) {
				LocalPackageInfo pinfo = new LocalPackageInfo(info);

				if (pinfo.isNeedBackup(filter)) {
					mPackageInfos.add(new LocalPackageInfo(info));
				}
			}

			notifyDataSetChangedSafe();
		}

		public void setEnableAll(boolean enable) {
			for (LocalPackageInfo info : mPackageInfos) {
				info.setEnable(enable);
			}

			notifyDataSetChangedSafe();
		}

		public void notifyDataSetChangedSafe() {
			if (CavanAndroid.isMainThread()) {
				notifyDataSetChanged();
			} else {
				runOnUiThread(new Runnable() {

					@Override
					public void run() {
						notifyDataSetChanged();
					}
				});
			}
		}

		@Override
		public int getCount() {
			return mPackageInfos.size();
		}

		@Override
		public Object getItem(int position) {
			return mPackageInfos.get(position);
		}

		@Override
		public long getItemId(int position) {
			return position;
		}

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			CavanCheckBox view;

			if (convertView == null) {
				view = new CavanCheckBox(MainActivity.this);
			} else {
				view = (CavanCheckBox) convertView;
			}

			LocalPackageInfo info = mPackageInfos.get(position);
			info.setupView(view, position);

			return view;
		}
	};

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case MSG_BACKUP_START:
				setBackupEnable(false);
				mProgressBar.setMax(mPackageInfos.size());
				mProgressBar.setProgress(0);
				break;

			case MSG_BACKUP_END:
				setBackupEnable(true);
				mTextViewState.setText(R.string.backup_complete);
				CavanAndroid.showToast(getApplicationContext(), R.string.backup_complete);
				break;

			case MSG_COPY_START:
				mTextViewState.setText(getResources().getString(R.string.backup_file, msg.obj));
				break;

			case MSG_COPY_END:
				mProgressBar.setProgress(msg.arg1);
				break;

			case MSG_CLEAR:
				mTextViewState.setText(getResources().getString(R.string.clear_directory, msg.obj));
				break;
			}
		}
	};

	private boolean isAllEnabled() {
		for (LocalPackageInfo info : mPackageInfos) {
			if (!info.isEnabled()) {
				return false;
			}
		}

		return true;
	}

	private void setBackupEnable(boolean enable) {
		mButtonStart.setEnabled(enable);
		mCheckBoxBackupSysApp.setEnabled(enable);

		enable = !enable;
		mButtonStop.setEnabled(enable);
		mProgressBar.setEnabled(enable);
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_apk_backup);

		mPackageManager = getPackageManager();

		mProgressBar = (ProgressBar) findViewById(R.id.progressBar);
		mTextViewState = (TextView) findViewById(R.id.textViewState);

		mCheckBoxSelectAll = (CavanCheckBox) findViewById(R.id.checkBoxSelectAll);
		mCheckBoxSelectAll.setOnCheckedChangeListener(this);

		mCheckBoxBackupSysApp = (CheckBox) findViewById(R.id.checkBoxBackupSysApp);
		mCheckBoxBackupSysApp.setOnCheckedChangeListener(this);

		mCheckBoxClearBeforeBackup = (CheckBox) findViewById(R.id.checkBoxClearBeforeBackup);

		mButtonStart = (Button) findViewById(R.id.buttonStart);
		mButtonStart.setOnClickListener(this);

		mButtonStop = (Button) findViewById(R.id.buttonStop);
		mButtonStop.setEnabled(false);
		mButtonStop.setOnClickListener(this);

		mListViewApps = (ListView) findViewById(R.id.listViewApps);
		mListViewApps.setAdapter(mAdapter);

		mEditTextSearch = (EditText) findViewById(R.id.editTextSearch);
		mEditTextSearch.addTextChangedListener(this);

		mAdapter.updateData();
		setBackupEnable(true);

		CavanAndroid.checkAndRequestPermissions(this, PERMISSIONS);
	}

	@Override
	public void onClick(View v) {
		if (v == mButtonStart) {
			if (mThread == null) {
				mThread = new BackupThread();
				mThread.start();
			}
		} else if (v == mButtonStop) {
			if (mThread != null) {
				mThread.setNeedStop();
			}

			mButtonStop.setEnabled(false);
		}
	}

	@Override
	public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
		switch (buttonView.getId()) {
		case R.id.checkBoxSelectAll:
			mAdapter.setEnableAll(isChecked);
			break;

		case R.id.checkBoxBackupSysApp:
			mAdapter.updateData();
			break;
		}
	}

	@Override
	public void beforeTextChanged(CharSequence s, int start, int count, int after) {
	}

	@Override
	public void onTextChanged(CharSequence s, int start, int before, int count) {
	}

	@Override
	public void afterTextChanged(Editable s) {
		mAdapter.updateData();
	}
}
