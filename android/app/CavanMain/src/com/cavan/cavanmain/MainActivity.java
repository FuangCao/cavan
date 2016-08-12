package com.cavan.cavanmain;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;
import java.util.Calendar;

import android.app.Notification.Builder;
import android.app.NotificationManager;
import android.graphics.Color;
import android.graphics.PixelFormat;
import android.media.Ringtone;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.EditTextPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.preference.RingtonePreference;
import android.view.Gravity;
import android.view.WindowManager;
import android.view.WindowManager.LayoutParams;
import android.widget.TextView;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanNetworkClient;
import com.cavan.cavanjni.CavanJni;
import com.cavan.cavanjni.CavanServicePreference;

public class MainActivity extends PreferenceActivity implements OnPreferenceChangeListener {

	public static final String KEY_IP_ADDRESS = "ip_address";
	public static final String KEY_FLOAT_TIMER = "float_timer";
	public static final String KEY_RED_PACKET_NOTIFY_TEST = "red_packet_notify_test";
	public static final String KEY_RED_PACKET_NOTIFY_RINGTONE = "red_packet_notify_ringtone";
	public static final String KEY_TCP_DD = "tcp_dd";
	public static final String KEY_FTP = "ftp";
	public static final String KEY_WEB_PROXY = "web_proxy";

	private static int sLastSecond;
	private static TextView sTimeView;
	private static Runnable mRunnableTime = new Runnable() {

		@Override
		public void run() {
			if (sTimeView != null) {
				Calendar calendar = Calendar.getInstance();
				int second = calendar.get(Calendar.SECOND);
				if (second == sLastSecond) {
					sTimeView.postDelayed(this, 100);
				} else {
					sLastSecond = second;
					sTimeView.postDelayed(this, 1000);

					int hour = calendar.get(Calendar.HOUR_OF_DAY);
					int minute = calendar.get(Calendar.MINUTE);

					sTimeView.setText(String.format("%02d:%02d:%02d", hour, minute, second));
				}
			}
		}
	};

	private File mFileBin;
	private Preference mPreferenceIpAddress;
	private CheckBoxPreference mPreferenceFloatTime;
	private EditTextPreference mPreferenceRedPacketNotifyTest;
	private RingtonePreference mPreferenceRedPacketNotifyRingtone;
	private CavanServicePreference mPreferenceTcpDd;
	private CavanServicePreference mPreferenceFtp;
	private CavanServicePreference mPreferenceWebProxy;

	@SuppressWarnings("deprecation")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.cavan_service);

		mPreferenceIpAddress = findPreference(KEY_IP_ADDRESS);

		mPreferenceFloatTime = (CheckBoxPreference) findPreference(KEY_FLOAT_TIMER);
		setDesktopFloatTimerEnable(mPreferenceFloatTime.isChecked());
		mPreferenceFloatTime.setOnPreferenceChangeListener(this);

		mPreferenceRedPacketNotifyTest = (EditTextPreference) findPreference(KEY_RED_PACKET_NOTIFY_TEST);
		String text = mPreferenceRedPacketNotifyTest.getText();
		if (text == null || text.isEmpty()) {
			mPreferenceRedPacketNotifyTest.setText("支付宝红包口令: 11223344");
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
	}

	@Override
	protected void onDestroy() {
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
		WindowManager manager = (WindowManager) getApplication().getSystemService(WINDOW_SERVICE);

		if (enable) {
			LayoutParams params = new LayoutParams();

			params.type = LayoutParams.TYPE_PHONE;
			params.format = PixelFormat.RGBA_8888;
			params.flags = LayoutParams.FLAG_NOT_FOCUSABLE;
			params.gravity = Gravity.RIGHT | Gravity.TOP;
			params.x = 0;
			params.y = 0;
			params.width = WindowManager.LayoutParams.WRAP_CONTENT;
			params.height = WindowManager.LayoutParams.WRAP_CONTENT;

			TextView view = new TextView(getApplicationContext());
			view.setBackgroundColor(Color.BLUE);
			view.setTextColor(Color.YELLOW);

			manager.addView(view, params);
			sTimeView = view;

			sLastSecond = -1;
			sTimeView.post(mRunnableTime);
		} else if (sTimeView != null) {
			manager.removeView(sTimeView);
			sTimeView = null;
		}

		return true;
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
			NotificationManager manager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
			if (manager != null) {
				String content = "CFA8888" + ": " + object;

				Builder builder = new Builder(this)
					.setSmallIcon(R.drawable.ic_launcher)
					.setAutoCancel(true)
					.setContentTitle("红包提醒测试")
					.setTicker(content)
					.setContentText(content);

				manager.notify(RedPacketListenerService.NOTIFY_TEST, builder.build());
			}
		} else if (preference == mPreferenceRedPacketNotifyRingtone) {
			updateRingtoneSummary((String) object);
		}

		return true;
	}
}
