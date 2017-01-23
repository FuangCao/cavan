package com.cavan.cavanapkbackup;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanFile;

public class MainActivity extends Activity implements OnClickListener, OnCheckedChangeListener {

	private static final CavanFile sOutDir = new CavanFile(Environment.getExternalStorageDirectory(), "cavan-apk-backup");

	private static final int MSG_BACKUP_START = 1;
	private static final int MSG_BACKUP_END = 2;
	private static final int MSG_COPY_START = 3;
	private static final int MSG_COPY_END = 4;
	private static final int MSG_CLEAR = 5;

	private ProgressBar mProgressBar;
	private TextView mTextViewState;
	private Button mButtonStart;
	private Button mButtonStop;
	private ListView mListViewApps;
	private CheckBox mCheckBoxSelectAll;
	private CheckBox mCheckBoxBackupSysApp;
	private CheckBox mCheckBoxClearBeforeBackup;

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

		private boolean mEnable;
		private PackageInfo mPackageInfo;

		public LocalPackageInfo(PackageInfo info) {
			mPackageInfo = info;
			mEnable = mCheckBoxSelectAll.isChecked();
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

		public CharSequence getApplicationLabel() {
			return mPackageManager.getApplicationLabel(getApplicationInfo());
		}

		public String getPackageName() {
			return getApplication().getPackageName();
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

			CharSequence label = getApplicationLabel();
			if (label == null) {
				builder.append(getPackageName());
			} else {
				builder.append(CavanFile.replaceInvalidFilenameChar(label.toString(), '_'));
			}

			builder.append('-');
			builder.append(getVersionName());
			builder.append(".apk");

			return builder.toString();
		}

		public void setupView(CheckBox view) {
			view.setText(getApplicationLabel());
			view.setChecked(mEnable);
			view.setOnCheckedChangeListener(this);
		}

		@Override
		public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
			mEnable = isChecked;
		}
	}

	public class LocalAdapter extends BaseAdapter {

		public void updateData() {
			mPackageInfos.clear();

			for (PackageInfo info : mPackageManager.getInstalledPackages(0)) {
				if (isNeedBackup(info)) {
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

		private boolean isNeedBackup(PackageInfo info) {
			if (mCheckBoxBackupSysApp.isChecked()) {
				return true;
			}

			return (info.applicationInfo.flags & ApplicationInfo.FLAG_SYSTEM) == 0;
		}

		public void notifyDataSetChangedSafe() {
			if (CavanAndroid.isMainThread()) {
				notifyDataSetChanged();
			} else {
				mListViewApps.post(new Runnable() {

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
			CheckBox view;

			if (convertView == null) {
				view = new CheckBox(MainActivity.this);
			} else {
				view = (CheckBox) convertView;
			}

			LocalPackageInfo info = mPackageInfos.get(position);
			info.setupView(view);

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

	private void setBackupEnable(boolean enable) {
		mButtonStart.setEnabled(enable);
		mCheckBoxSelectAll.setEnabled(enable);
		mCheckBoxBackupSysApp.setEnabled(enable);
		mCheckBoxClearBeforeBackup.setEnabled(enable);

		enable = !enable;
		mButtonStop.setEnabled(enable);
		mProgressBar.setEnabled(enable);
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mPackageManager = getPackageManager();

		mProgressBar = (ProgressBar) findViewById(R.id.progressBar);
		mTextViewState = (TextView) findViewById(R.id.textViewState);

		mCheckBoxSelectAll = (CheckBox) findViewById(R.id.checkBoxSelectAll);
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
		mAdapter.updateData();

		setBackupEnable(true);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
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
}
