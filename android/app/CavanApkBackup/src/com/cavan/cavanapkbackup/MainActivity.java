package com.cavan.cavanapkbackup;

import java.io.File;
import java.util.List;

import android.app.Activity;
import android.content.pm.ApplicationInfo;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanFile;

public class MainActivity extends Activity implements OnClickListener {

	private static final CavanFile sOutDir = new CavanFile(Environment.getExternalStorageDirectory(), "cavan-apk-backup");

	private static final int MSG_BACKUP_START = 1;
	private static final int MSG_BACKUP_END = 2;
	private static final int MSG_COPY_START = 3;
	private static final int MSG_COPY_END = 4;
	private static final int MSG_CLEAR = 5;

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
				List<ApplicationInfo> infos = getPackageManager().getInstalledApplications(0);

				Message message = mHandler.obtainMessage(MSG_BACKUP_START, infos.size(), 0);
				message.sendToTarget();

				if (mCheckBoxClearBeforeBackup.isChecked()) {
					message = mHandler.obtainMessage(MSG_CLEAR, sOutDir.getPath());
					message.sendToTarget();
					sOutDir.clear();
				}

				int progress = 0;

				for (ApplicationInfo info : infos) {
					if (mNeedStop) {
						break;
					}

					if (mCheckBoxBackupSysApp.isChecked() || (info.flags & ApplicationInfo.FLAG_SYSTEM) == 0) {
						CavanFile outFile = new CavanFile(sOutDir, info.packageName + ".apk");

						message = mHandler.obtainMessage(MSG_COPY_START, outFile.getPath());
						message.sendToTarget();

						if (!doBackupFile(new File(info.sourceDir), outFile)) {
							break;
						}
					}

					message = mHandler.obtainMessage(MSG_COPY_END, ++progress, 0);
					message.sendToTarget();
				}

				mHandler.sendEmptyMessage(MSG_BACKUP_END);
			}

			mThread = null;
		}
	}

	private ProgressBar mProgressBar;
	private TextView mTextViewState;
	private Button mButtonStart;
	private Button mButtonStop;
	private CheckBox mCheckBoxBackupSysApp;
	private CheckBox mCheckBoxClearBeforeBackup;

	private BackupThread mThread;

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case MSG_BACKUP_START:
				mButtonStart.setEnabled(false);
				mButtonStop.setEnabled(true);

				mProgressBar.setMax(msg.arg1);
				mProgressBar.setProgress(0);
				break;

			case MSG_BACKUP_END:
				mButtonStart.setEnabled(true);
				mButtonStop.setEnabled(false);

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

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mProgressBar = (ProgressBar) findViewById(R.id.progressBar);
		mTextViewState = (TextView) findViewById(R.id.textViewState);
		mCheckBoxBackupSysApp = (CheckBox) findViewById(R.id.checkBoxBackupSysApp);
		mCheckBoxClearBeforeBackup = (CheckBox) findViewById(R.id.checkBoxClearBeforeBackup);

		mButtonStart = (Button) findViewById(R.id.buttonStart);
		mButtonStart.setOnClickListener(this);

		mButtonStop = (Button) findViewById(R.id.buttonStop);
		mButtonStop.setEnabled(false);
		mButtonStop.setOnClickListener(this);
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
		}
	}
}
