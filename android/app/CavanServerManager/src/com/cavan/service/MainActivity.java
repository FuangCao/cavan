package com.cavan.service;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import com.cavan.service.CavanService.ServiceBinder;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.preference.CheckBoxPreference;
import android.preference.EditTextPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceActivity;
import android.util.Log;
import android.view.Menu;

public class MainActivity extends PreferenceActivity implements OnPreferenceChangeListener {
	private static final String TAG = "Cavan";
	private static final String KEY_ENABLE_TCP_DD = "enable_tcp_dd";
	private static final String KEY_TCP_DD_PORT = "tcp_dd_port";
	private static final String KEY_ENABLE_FTP = "enable_ftp";
	private static final String KEY_FTP_PORT = "ftp_port";

	private File mFileCavanMain;

	private CheckBoxPreference mPreferenceEnableTcpDd;
	private EditTextPreference mPreferenceTcpDdPort;
	private TcpDdService mTcpDdService;

	private CheckBoxPreference mPreferenceEnableFtp;
	private EditTextPreference mPreferenceFtpPort;
	private FtpService mFtpService;

	private ServiceConnection mTcpDdConnection = new ServiceConnection() {
		@Override
		public void onServiceDisconnected(ComponentName arg0) {
			Log.d(TAG, "onServiceDisconnected");
			mTcpDdService = null;
		}

		@Override
		public void onServiceConnected(ComponentName arg0, IBinder arg1) {
			Log.d(TAG, "onServiceConnected");
			ServiceBinder binder = (ServiceBinder) arg1;
			mTcpDdService = (TcpDdService) binder.getService();
			updateTcpDdState(mTcpDdService.getState());

			String port = Integer.toString(mTcpDdService.getPort());
			mPreferenceTcpDdPort.setText(port);
			mPreferenceTcpDdPort.setSummary(port);
		}
	};

	private ServiceConnection mFtpConnection = new ServiceConnection() {
		@Override
		public void onServiceDisconnected(ComponentName arg0) {
			Log.d(TAG, "onServiceDisconnected");
			mFtpService = null;
		}

		@Override
		public void onServiceConnected(ComponentName arg0, IBinder arg1) {
			Log.d(TAG, "onServiceConnected");
			ServiceBinder binder = (ServiceBinder) arg1;
			mFtpService = (FtpService) binder.getService();
			updateFtpState(mFtpService.getState());

			String port = Integer.toString(mFtpService.getPort());
			mPreferenceFtpPort.setText(port);
			mPreferenceFtpPort.setSummary(port);
		}
	};

	private IntentFilter mFilter;

	private BroadcastReceiver mReceiver = new BroadcastReceiver() {
		@Override
			public void onReceive(Context arg0, Intent arg1) {
				String action = arg1.getAction();
				Log.d(TAG, "action = " + action);

				if (action.equals(TcpDdService.ACTION_SERVICE_STATE_CHANGE)) {
					int state = arg1.getIntExtra("state", -1);
					updateTcpDdState(state);
				} else if (action.equals(FtpService.ACTION_SERVICE_STATE_CHANGE)) {
					int state = arg1.getIntExtra("state", -1);
					updateFtpState(state);
				}
		}
	};

	private void updateTcpDdState(int state) {
		switch (state) {
		case CavanService.SERVICE_STATE_RUNNING:
			mPreferenceEnableTcpDd.setChecked(true);
			mPreferenceEnableTcpDd.setEnabled(true);
			mPreferenceEnableTcpDd.setSummary(R.string.service_running);
			mPreferenceTcpDdPort.setEnabled(false);
			break;
		case CavanService.SERVICE_STATE_STOPPED:
			mPreferenceEnableTcpDd.setChecked(false);
			mPreferenceEnableTcpDd.setEnabled(true);
			mPreferenceEnableTcpDd.setSummary(R.string.service_stopped);
			mPreferenceTcpDdPort.setEnabled(true);
			break;
		}
	}

	private void updateFtpState(int state) {
		switch (state) {
		case CavanService.SERVICE_STATE_RUNNING:
			mPreferenceEnableFtp.setChecked(true);
			mPreferenceEnableFtp.setEnabled(true);
			mPreferenceEnableFtp.setSummary(R.string.service_running);
			mPreferenceFtpPort.setEnabled(false);
			break;
		case CavanService.SERVICE_STATE_STOPPED:
			mPreferenceEnableFtp.setChecked(false);
			mPreferenceEnableFtp.setEnabled(true);
			mPreferenceEnableFtp.setSummary(R.string.service_stopped);
			mPreferenceFtpPort.setEnabled(true);
			break;
		}
	}

	public MainActivity() {
		super();

		mFilter = new IntentFilter();
		mFilter.addAction(TcpDdService.ACTION_SERVICE_STATE_CHANGE);
		mFilter.addAction(FtpService.ACTION_SERVICE_STATE_CHANGE);
	}

	@Override
	protected void onPause() {
		unregisterReceiver(mReceiver);
		super.onPause();
	}

	@Override
	protected void onResume() {
		registerReceiver(mReceiver, mFilter);
		super.onResume();
	}

	@SuppressWarnings("deprecation")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		mFileCavanMain = new File(getFilesDir(), "cavan-main");
		try {
			prepareCavanMain(R.raw.cavan, mFileCavanMain);
		} catch (IOException e) {
			e.printStackTrace();
		}

		addPreferencesFromResource(R.xml.main);

		mPreferenceEnableTcpDd = (CheckBoxPreference) findPreference(KEY_ENABLE_TCP_DD);
		mPreferenceEnableTcpDd.setOnPreferenceChangeListener(this);
		mPreferenceTcpDdPort = (EditTextPreference) findPreference(KEY_TCP_DD_PORT);
		mPreferenceTcpDdPort.setOnPreferenceChangeListener(this);
		
		mPreferenceEnableFtp = (CheckBoxPreference) findPreference(KEY_ENABLE_FTP);
		mPreferenceEnableFtp.setOnPreferenceChangeListener(this);
		mPreferenceFtpPort = (EditTextPreference) findPreference(KEY_FTP_PORT);
		mPreferenceFtpPort.setOnPreferenceChangeListener(this);

		bindService(new Intent(this, TcpDdService.class), mTcpDdConnection, BIND_AUTO_CREATE);
		bindService(new Intent(this, FtpService.class), mFtpConnection, BIND_AUTO_CREATE);
	}

	@Override
	protected void onDestroy() {
		Log.d(TAG, "onDestroy()");
		super.onDestroy();
		unbindService(mTcpDdConnection);
		unbindService(mFtpConnection);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.activity_main, menu);
		return true;
	}

	public boolean onPreferenceChange(Preference arg0, Object arg1) {
		if (arg0.equals(mPreferenceEnableTcpDd)) {
			mPreferenceEnableTcpDd.setEnabled(false);
			if (arg1.equals(true)) {
				mPreferenceEnableTcpDd.setSummary(R.string.service_opening);
				int port = Integer.valueOf(mPreferenceTcpDdPort.getText());
				mTcpDdService.start(mFileCavanMain.getAbsolutePath(), port);
			} else {
				mPreferenceEnableTcpDd.setSummary(R.string.service_stopping);
				mTcpDdService.stop();
			}
		} else if (arg0.equals(mPreferenceTcpDdPort)) {
			mPreferenceTcpDdPort.setSummary((CharSequence) arg1);
			return true;
		} else if (arg0.equals(mPreferenceEnableFtp)) {
			mPreferenceEnableFtp.setEnabled(false);
			if (arg1.equals(true)) {
				mPreferenceEnableFtp.setSummary(R.string.service_opening);
				int port = Integer.valueOf(mPreferenceFtpPort.getText());
				mFtpService.start(mFileCavanMain.getAbsolutePath(), port);
			} else {
				mPreferenceEnableFtp.setSummary(R.string.service_stopping);
				mFtpService.stop();
			}
		} else if (arg0.equals(mPreferenceFtpPort)) {
			mPreferenceFtpPort.setSummary((CharSequence) arg1);
			return true;
		}

		return false;
	}

	private boolean prepareCavanMain(int id, File file) throws IOException {
		if (file.exists()) {
			return true;
		}

		Log.d(TAG, "Extra file " + file.getAbsolutePath());

		InputStream inputStream = getResources().openRawResource(id);

		FileOutputStream outputStream;
		try {
			outputStream = new FileOutputStream(file);
		} catch (FileNotFoundException e) {
			e.printStackTrace();
			inputStream.close();
			return false;
		}

		int length;
		byte []buff = new byte[1024];

		try {
			while ((length = inputStream.read(buff)) > 0) {
				outputStream.write(buff, 0, length);
			}
		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			outputStream.close();
			inputStream.close();
		}

		file.setExecutable(true);

		return true;
	}
}
