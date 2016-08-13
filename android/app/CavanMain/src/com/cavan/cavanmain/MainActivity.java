package com.cavan.cavanmain;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;

import android.app.Notification.Builder;
import android.app.NotificationManager;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.media.Ringtone;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.preference.CheckBoxPreference;
import android.preference.EditTextPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.preference.RingtonePreference;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanNetworkClient;
import com.cavan.cavanjni.CavanJni;
import com.cavan.cavanjni.CavanServicePreference;
import com.cavan.java.CavanString;

public class MainActivity extends PreferenceActivity implements OnPreferenceChangeListener {

	public static final String KEY_IP_ADDRESS = "ip_address";
	public static final String KEY_FLOAT_TIMER = "float_timer";
	public static final String KEY_RED_PACKET_NOTIFY_TEST = "red_packet_notify_test";
	public static final String KEY_RED_PACKET_NOTIFY_RINGTONE = "red_packet_notify_ringtone";
	public static final String KEY_TCP_DD = "tcp_dd";
	public static final String KEY_FTP = "ftp";
	public static final String KEY_WEB_PROXY = "web_proxy";

	private File mFileBin;
	private Preference mPreferenceIpAddress;
	private CheckBoxPreference mPreferenceFloatTime;
	private EditTextPreference mPreferenceRedPacketNotifyTest;
	private RingtonePreference mPreferenceRedPacketNotifyRingtone;
	private CavanServicePreference mPreferenceTcpDd;
	private CavanServicePreference mPreferenceFtp;
	private CavanServicePreference mPreferenceWebProxy;

	private IFloatTimerService mFloatTimerService;
	private ServiceConnection mFloatTimerConnection = new ServiceConnection() {

		@Override
		public void onServiceDisconnected(ComponentName arg0) {
			mFloatTimerService = null;
		}

		@Override
		public void onServiceConnected(ComponentName component, IBinder binder) {
			mFloatTimerService = IFloatTimerService.Stub.asInterface(binder);
			setDesktopFloatTimerEnable(mPreferenceFloatTime.isChecked());
		}
	};

	@SuppressWarnings("deprecation")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.cavan_service);

		mPreferenceIpAddress = findPreference(KEY_IP_ADDRESS);

		mPreferenceFloatTime = (CheckBoxPreference) findPreference(KEY_FLOAT_TIMER);
		mPreferenceFloatTime.setOnPreferenceChangeListener(this);

		mPreferenceRedPacketNotifyTest = (EditTextPreference) findPreference(KEY_RED_PACKET_NOTIFY_TEST);
		String text = mPreferenceRedPacketNotifyTest.getText();
		if (text == null || text.isEmpty()) {
			mPreferenceRedPacketNotifyTest.setText("支付宝红包口令: 12345678");
		}

		mPreferenceRedPacketNotifyTest.setPositiveButtonText(R.string.text_test);
		mPreferenceRedPacketNotifyTest.setOnPreferenceChangeListener(this);

		mPreferenceRedPacketNotifyRingtone = (RingtonePreference) findPreference(KEY_RED_PACKET_NOTIFY_RINGTONE);
		text = mPreferenceRedPacketNotifyRingtone.getPreferenceManager().getSharedPreferences().getString(KEY_RED_PACKET_NOTIFY_RINGTONE, null);
		if (text != null) {
			updateRingtoneSummary(text);
		}

		mPreferenceRedPacketNotifyRingtone.setRingtoneType(RingtoneManager.TYPE_NOTIFICATION);
		mPreferenceRedPacketNotifyRingtone.setOnPreferenceChangeListener(this);

		mPreferenceTcpDd = (CavanServicePreference) findPreference(KEY_TCP_DD);
		mPreferenceFtp = (CavanServicePreference) findPreference(KEY_FTP);
		mPreferenceWebProxy = (CavanServicePreference) findPreference(KEY_WEB_PROXY);

		updateIpAddressStatus();

		mFileBin = getDir("bin", 0777);
		if (mFileBin == null) {
			CavanAndroid.logE("Failed to getDir bin");
		} else {
			CavanJni.appendPathEnv(mFileBin.getPath());

			new Thread() {

				@Override
				public void run() {
					CavanAndroid.logD("releaseCavanMain " + (releaseCavanMain() ? "OK" : "Failed"));
				}
			}.start();
		}

		Intent service = new Intent(this, FloatTimerService.class);

		startService(service);
		bindService(service, mFloatTimerConnection, 0);
	}

	@Override
	protected void onDestroy() {
		unbindService(mFloatTimerConnection);

		mPreferenceTcpDd.unbindService(this);
		mPreferenceFtp.unbindService(this);
		mPreferenceWebProxy.unbindService(this);

		super.onDestroy();
	}

	@SuppressWarnings("deprecation")
	private boolean releaseCavanMain() {
		File fileCavanMain = new File(mFileBin, "cavan-main");
		if (fileCavanMain.canExecute()) {
			return true;
		}

		for (String abi : new String[] { Build.CPU_ABI, Build.CPU_ABI2 }) {
			String filename = "cavan-main" + "." + abi;

			if (releaseAsset(filename, fileCavanMain)) {
				fileCavanMain.setExecutable(true, false);
				return true;
			}
		}

		return false;
	}

	private boolean releaseAsset(InputStream inStream, OutputStream outStream) {
		byte[] buff = new byte[1024];

		try {
			while (true) {
				int length = inStream.read(buff);
				if (length < 1) {
					break;
				}

				outStream.write(buff, 0, length);
			}

			return true;
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
	}

	private boolean releaseAsset(String filename, File outFile) {
		CavanAndroid.logD("releaseAsset: " + filename + " => " + outFile.getPath());

		InputStream inStream = null;
		OutputStream outStream = null;

		try {
			inStream = getAssets().open(filename);
			outStream = new FileOutputStream(outFile);
			return releaseAsset(inStream, outStream);
		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			if (inStream != null) {
				try {
					inStream.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}

			if (outStream != null) {
				try {
					outStream.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}

		return false;
	}

	private void updateIpAddressStatus() {
		InetAddress address = CavanNetworkClient.getIpAddress();
		if (address != null) {
			mPreferenceIpAddress.setSummary(address.getHostAddress());
		} else {
			mPreferenceIpAddress.setSummary(R.string.text_unknown);
		}
	}

	private boolean setDesktopFloatTimerEnable(boolean enable) {
		if (mFloatTimerService == null) {
			return false;
		}

		try {
			return mFloatTimerService.setEnable(enable);
		} catch (RemoteException e) {
			e.printStackTrace();
		}

		return false;
	}

	@SuppressWarnings("deprecation")
	public void updateRingtoneSummary(String uri) {
		String summary;

		Ringtone ringtone = RingtoneManager.getRingtone(this, Uri.parse(uri));
		if (ringtone == null) {
			getPreferenceManager().getSharedPreferences().edit().remove(KEY_RED_PACKET_NOTIFY_RINGTONE);
			summary = null;
		} else {
			summary = ringtone.getTitle(this);
			if (summary == null) {
				summary = uri;
			}
		}

		mPreferenceRedPacketNotifyRingtone.setSummary(summary);
	}

	@SuppressWarnings("deprecation")
	@Override
	public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
		if (preference == mPreferenceIpAddress) {
			updateIpAddressStatus();
		}

		return super.onPreferenceTreeClick(preferenceScreen, preference);
	}

	@Override
	public boolean onPreferenceChange(Preference preference, Object object) {
		if (preference == mPreferenceFloatTime) {
			return setDesktopFloatTimerEnable((boolean) object);
		} else if (preference == mPreferenceRedPacketNotifyTest) {
			if (CavanAndroid.isNotificationListenerEnabled(this, RedPacketListenerService.class)) {
				NotificationManager manager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
				if (manager != null) {
					String text = (String) object;
					if (CavanString.hasChineseChar(text) && text.matches("[::]") == false) {
						text = "支付宝红包口令: " + text;
					}

					Builder builder = new Builder(this)
						.setSmallIcon(R.drawable.ic_launcher)
						.setAutoCancel(true)
						.setContentTitle("红包提醒测试")
						.setTicker("CFA8888: " + text)
						.setContentText(text);

					manager.notify(RedPacketListenerService.NOTIFY_TEST, builder.build());
				}
			} else {
				Intent intent = new Intent("android.settings.ACTION_NOTIFICATION_LISTENER_SETTINGS");
				startActivity(intent);

				CavanAndroid.showToastLong(this, "请打开通知读取权限");
			}
		} else if (preference == mPreferenceRedPacketNotifyRingtone) {
			updateRingtoneSummary((String) object);
		}

		return true;
	}
}
