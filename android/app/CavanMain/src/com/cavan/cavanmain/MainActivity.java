package com.cavan.cavanmain;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;

import android.app.Notification.Builder;
import android.app.NotificationManager;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
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
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.preference.RingtonePreference;
import android.text.InputType;

import com.cavan.android.CavanAndroid;
import com.cavan.cavanjni.CavanJni;
import com.cavan.cavanjni.CavanServicePreference;
import com.cavan.java.CavanJava;
import com.cavan.java.CavanString;

public class MainActivity extends PreferenceActivity implements OnPreferenceChangeListener {

	public static final String ACTION_CODE_ADD = "cavan.intent.action.ACTION_CODE_ADD";
	public static final String ACTION_CODE_TEST = "cavan.intent.action.ACTION_CODE_TEST";
	public static final String ACTION_CODE_REMOVE = "cavan.intent.action.ACTION_CODE_REMOVE";
	public static final String ACTION_CODE_COMMIT = "cavan.intent.action.ACTION_CODE_COMMIT";
	public static final String ACTION_CODE_RECEIVED = "cavan.intent.action.ACTION_CODE_RECEIVED";
	public static final String ACTION_CONTENT_RECEIVED = "cavan.intent.action.ACTION_CONTENT_RECEIVED";
	public static final String ACTION_TEXT_RECEIVED = "cavan.intent.action.ACTION_TEXT_RECEIVED";
	public static final String ACTION_WAN_UPDATED = "cavan.intent.action.ACTION_WAN_UPDATED";
	public static final String ACTION_BRIDGE_UPDATED = "cavan.intent.action.ACTION_BRIDGE_UPDATED";

	public static final String KEY_IP_ADDRESS = "ip_address";
	public static final String KEY_AUTO_UNLOCK = "auto_unlock";
	public static final String KEY_AUTO_COMMIT = "auto_commit";
	public static final String KEY_AUTO_UNPACK = "auto_unpack";
	public static final String KEY_LISTEN_CLIP = "listen_clip";
	public static final String KEY_LISTEN_CLICK = "listen_click";
	public static final String KEY_FLOAT_TIMER = "float_timer";
	public static final String KEY_LAN_SHARE = "lan_share";
	public static final String KEY_LAN_TEST = "lan_test";
	public static final String KEY_WAN_SHARE = "wan_share";
	public static final String KEY_WAN_TEST = "wan_test";
	public static final String KEY_WAN_IP = "wan_ip";
	public static final String KEY_WAN_PORT = "wan_port";
	public static final String KEY_MESSAGE_SHOW = "message_show";
	public static final String KEY_COMMIT_AHEAD = "commit_ahead";
	public static final String KEY_AUTO_OPEN_APP = "auto_open_app";
	public static final String KEY_INPUT_METHOD_SELECT = "input_method_select";
	public static final String KEY_PERMISSION_SETTINGS = "permission_settings";
	public static final String KEY_RED_PACKET_CODE_SEND = "red_packet_code_send";
	public static final String KEY_RED_PACKET_NOTIFY_TEST = "red_packet_notify_test";
	public static final String KEY_RED_PACKET_NOTIFY_RINGTONE = "red_packet_notify_ringtone";
	public static final String KEY_TCP_BRIDGE = "tcp_bridge";
	public static final String KEY_TCP_BRIDGE_SETTING = "tcp_bridge_setting";
	public static final String KEY_FTP = "ftp";
	public static final String KEY_TCP_DD = "tcp_dd";
	public static final String KEY_WEB_PROXY = "web_proxy";
	public static final String KEY_TCP_REPEATER = "tcp_repeater";

	static {
		CavanAndroid.TAG = "CavanMain";
	}

	public static boolean isFloatTimerEnabled(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_FLOAT_TIMER);
	}

	public static int getAutoCommitCount(Context context) {
		String text = CavanAndroid.getPreference(context, KEY_AUTO_COMMIT, "1");

		try {
			return Integer.parseInt(text);
		} catch (Exception e) {
			return 1;
		}
	}

	public static boolean isAutoUnpackEnabled(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_AUTO_UNPACK);
	}

	public static boolean isListenClipEnabled(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_LISTEN_CLIP);
	}

	public static boolean isListenClickEnabled(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_LISTEN_CLICK);
	}

	public static boolean isLanShareEnabled(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_LAN_SHARE);
	}

	public static boolean isWanShareEnabled(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_WAN_SHARE);
	}

	public static String getWanShareIpAddress(Context context) {
		return CavanAndroid.getPreference(context, KEY_WAN_IP, null);
	}

	public static boolean isTcpBridgeEnabled(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_TCP_BRIDGE);
	}

	public static String getTcpBridgeSetting(Context context) {
		return CavanAndroid.getPreference(context, KEY_TCP_BRIDGE_SETTING, null);
	}

	public static int getWanSharePort(Context context) {
		String text = CavanAndroid.getPreference(context, KEY_WAN_PORT, null);

		try {
			if (text != null) {
				return Integer.parseInt(text);
			}
		} catch (Exception e) {
			e.printStackTrace();
		}

		return -1;
	}

	public static boolean isAutoOpenAppEnabled(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_AUTO_OPEN_APP);
	}

	public static int getCommitAhead(Context context) {
		String text = CavanAndroid.getPreference(context, KEY_COMMIT_AHEAD, null);

		try {
			if (text != null) {
				return Integer.parseInt(text);
			}
		} catch (Exception e) {
			e.printStackTrace();
		}

		return 5;
	}

	private File mFileBin;
	private Preference mPreferenceIpAddress;
	private Preference mPreferenceInputMethodSelect;
	private CheckBoxPreference mPreferenceFloatTime;
	private CheckBoxPreference mPreferenceAutoUnlock;
	private Preference mPreferencePermissionSettings;
	private Preference mPreferenceMessageShow;
	private ListPreference mPreferenceAutoCommit;
	private EditTextPreference mPreferenceRedPacketCodeSend;
	private EditTextPreference mPreferenceRedPacketNotifyTest;
	private RingtonePreference mPreferenceRedPacketNotifyRingtone;
	private CavanServicePreference mPreferenceTcpDd;
	private CavanServicePreference mPreferenceFtp;
	private CavanServicePreference mPreferenceWebProxy;
	private CavanServicePreference mPreferenceTcpRepeater;
	private CheckBoxPreference mPreferenceWanShare;
	private EditTextPreference mPreferenceWanIp;
	private EditTextPreference mPreferenceWanPort;
	private Preference mPreferenceLanTest;
	private Preference mPreferenceWanTest;
	private CheckBoxPreference mPreferenceTcpBridge;
	private EditTextPreference mPreferenceTcpBridgeSetting;
	private ListPreference mPreferenceCommitAhead;

	private IFloatMessageService mFloatMessageService;
	private ServiceConnection mFloatMessageConnection = new ServiceConnection() {

		@Override
		public void onServiceDisconnected(ComponentName arg0) {
			mFloatMessageService = null;
		}

		@Override
		public void onServiceConnected(ComponentName component, IBinder binder) {
			mFloatMessageService = IFloatMessageService.Stub.asInterface(binder);
			setDesktopFloatTimerEnable(mPreferenceFloatTime.isChecked());
		}
	};

	private BroadcastReceiver mReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();

			CavanAndroid.eLog("action = " + action);

			switch (action) {
			case ACTION_WAN_UPDATED:
				switch (intent.getIntExtra("state", 0)) {
				case R.string.text_wan_connecting:
					mPreferenceWanShare.setSummary(R.string.text_connecting);
					break;

				case R.string.text_wan_connected:
					mPreferenceWanShare.setSummary(R.string.text_connected);
					break;

				case R.string.text_wan_disconnected:
					mPreferenceWanShare.setSummary(R.string.text_disconnected);
					break;
				}
				break;

			case ACTION_BRIDGE_UPDATED:
				switch (intent.getIntExtra("state", 0)) {
				case R.string.text_tcp_bridge_running:
					mPreferenceTcpBridge.setSummary(R.string.text_running);
					break;

				case R.string.text_tcp_bridge_stopped:
					mPreferenceTcpBridge.setSummary(R.string.text_stopped);
					break;

				case R.string.text_tcp_bridge_exit:
					mPreferenceTcpBridge.setSummary(R.string.text_exit);
					break;
				}
				break;
			}
		}
	};

	@SuppressWarnings("deprecation")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.cavan_service);

		mPreferenceIpAddress = findPreference(KEY_IP_ADDRESS);
		mPreferenceInputMethodSelect = findPreference(KEY_INPUT_METHOD_SELECT);
		mPreferenceLanTest = findPreference(KEY_LAN_TEST);
		mPreferenceWanTest = findPreference(KEY_WAN_TEST);

		mPreferenceAutoUnlock = (CheckBoxPreference) findPreference(KEY_AUTO_UNLOCK);
		mPreferenceAutoUnlock.setOnPreferenceChangeListener(this);

		mPreferenceMessageShow = findPreference(KEY_MESSAGE_SHOW);
		mPreferenceMessageShow.setIntent(CavanMessageActivity.getIntent(this));

		mPreferenceAutoCommit = (ListPreference) findPreference(KEY_AUTO_COMMIT);
		mPreferenceAutoCommit.setSummary(mPreferenceAutoCommit.getEntry());
		mPreferenceAutoCommit.setOnPreferenceChangeListener(this);

		mPreferencePermissionSettings = findPreference(KEY_PERMISSION_SETTINGS);
		mPreferencePermissionSettings.setIntent(PermissionSettingsActivity.getIntent(this));

		mPreferenceFloatTime = (CheckBoxPreference) findPreference(KEY_FLOAT_TIMER);
		mPreferenceFloatTime.setOnPreferenceChangeListener(this);

		mPreferenceWanShare = (CheckBoxPreference) findPreference(KEY_WAN_SHARE);
		mPreferenceWanShare.setOnPreferenceChangeListener(this);

		mPreferenceWanIp = (EditTextPreference) findPreference(KEY_WAN_IP);
		String text = mPreferenceWanIp.getText();
		if (text == null || text.isEmpty()) {
			mPreferenceWanIp.setText("127.0.0.1");
		}
		mPreferenceWanIp.setSummary(mPreferenceWanIp.getText());
		mPreferenceWanIp.setOnPreferenceChangeListener(this);

		mPreferenceWanPort = (EditTextPreference) findPreference(KEY_WAN_PORT);
		mPreferenceWanPort.getEditText().setInputType(InputType.TYPE_CLASS_NUMBER);
		text = mPreferenceWanPort.getText();
		if (text == null || text.isEmpty()) {
			mPreferenceWanPort.setText("8864");
		}
		mPreferenceWanPort.setSummary(mPreferenceWanPort.getText());
		mPreferenceWanPort.setOnPreferenceChangeListener(this);

		mPreferenceRedPacketNotifyTest = (EditTextPreference) findPreference(KEY_RED_PACKET_NOTIFY_TEST);
		mPreferenceRedPacketNotifyTest.setPositiveButtonText(R.string.text_test);
		text = mPreferenceRedPacketNotifyTest.getText();
		if (text == null || text.isEmpty()) {
			mPreferenceRedPacketNotifyTest.setText("支付宝红包口令：12345678");
		}
		mPreferenceRedPacketNotifyTest.setOnPreferenceChangeListener(this);

		mPreferenceRedPacketCodeSend = (EditTextPreference) findPreference(KEY_RED_PACKET_CODE_SEND);
		mPreferenceRedPacketCodeSend.setPositiveButtonText(R.string.text_send);
		mPreferenceRedPacketCodeSend.setOnPreferenceChangeListener(this);

		mPreferenceRedPacketNotifyRingtone = (RingtonePreference) findPreference(KEY_RED_PACKET_NOTIFY_RINGTONE);
		text = mPreferenceRedPacketNotifyRingtone.getPreferenceManager().getSharedPreferences().getString(KEY_RED_PACKET_NOTIFY_RINGTONE, null);
		if (text != null) {
			updateRingtoneSummary(text);
		}

		mPreferenceRedPacketNotifyRingtone.setRingtoneType(RingtoneManager.TYPE_NOTIFICATION);
		mPreferenceRedPacketNotifyRingtone.setOnPreferenceChangeListener(this);

		mPreferenceTcpBridge = (CheckBoxPreference) findPreference(KEY_TCP_BRIDGE);
		mPreferenceTcpBridge.setOnPreferenceChangeListener(this);

		mPreferenceTcpBridgeSetting = (EditTextPreference) findPreference(KEY_TCP_BRIDGE_SETTING);
		mPreferenceTcpBridgeSetting.setSummary(mPreferenceTcpBridgeSetting.getText());
		mPreferenceTcpBridgeSetting.setOnPreferenceChangeListener(this);

		mPreferenceFtp = (CavanServicePreference) findPreference(KEY_FTP);
		mPreferenceTcpDd = (CavanServicePreference) findPreference(KEY_TCP_DD);
		mPreferenceWebProxy = (CavanServicePreference) findPreference(KEY_WEB_PROXY);
		mPreferenceTcpRepeater = (CavanServicePreference) findPreference(KEY_TCP_REPEATER);

		mPreferenceCommitAhead = (ListPreference) findPreference(KEY_COMMIT_AHEAD);
		mPreferenceCommitAhead.setSummary(mPreferenceCommitAhead.getEntry());
		mPreferenceCommitAhead.setOnPreferenceChangeListener(this);

		updateIpAddressStatus();

		mFileBin = getDir("bin", 0777);
		if (mFileBin == null) {
			CavanAndroid.eLog("Failed to getDir bin");
		} else {
			CavanJni.appendPathEnv(mFileBin.getPath());

			new Thread() {

				@Override
				public void run() {
					CavanAndroid.dLog("releaseCavanMain " + (releaseCavanMain() ? "OK" : "Failed"));
				}
			}.start();
		}

		Intent service = FloatMessageService.buildIntent(this);

		startService(service);
		bindService(service, mFloatMessageConnection, 0);

		IntentFilter filter = new IntentFilter();
		filter.addAction(ACTION_WAN_UPDATED);
		filter.addAction(ACTION_BRIDGE_UPDATED);
		registerReceiver(mReceiver, filter);
	}

	@Override
	protected void onDestroy() {
		unregisterReceiver(mReceiver);
		unbindService(mFloatMessageConnection);

		mPreferenceFtp.unbindService(this);
		mPreferenceTcpDd.unbindService(this);
		mPreferenceWebProxy.unbindService(this);
		mPreferenceTcpRepeater.unbindService(this);

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
		CavanAndroid.dLog("releaseAsset: " + filename + " => " + outFile.getPath());

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
		InetAddress address = CavanJava.getIpAddress();
		if (address != null) {
			mPreferenceIpAddress.setSummary(address.getHostAddress());
		} else {
			mPreferenceIpAddress.setSummary(R.string.text_unknown);
		}
	}

	private boolean setDesktopFloatTimerEnable(boolean enable) {
		if (mFloatMessageService == null) {
			return false;
		}

		try {
			return mFloatMessageService.setTimerEnable(enable);
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
		} else if (preference == mPreferenceInputMethodSelect) {
			CavanAndroid.showInputMethodPicker(this);
		} else if (preference == mPreferenceLanTest) {
			if (mFloatMessageService != null) {
				try {
					mFloatMessageService.sendUdpCommand(FloatMessageService.NET_CMD_TEST);
				} catch (RemoteException e) {
					e.printStackTrace();
				}
			}
		} else if (preference == mPreferenceWanTest) {
			if (mFloatMessageService != null) {
				try {
					mFloatMessageService.sendTcpCommand(FloatMessageService.NET_CMD_TEST);
				} catch (RemoteException e) {
					e.printStackTrace();
				}
			}
		}

		return super.onPreferenceTreeClick(preferenceScreen, preference);
	}

	@Override
	public boolean onPreferenceChange(Preference preference, Object object) {
		if (preference == mPreferenceFloatTime) {
			return setDesktopFloatTimerEnable((boolean) object);
		} else if (preference == mPreferenceRedPacketCodeSend) {
			String text = (String) object;
			if (text != null) {
				for (String line : text.split("\n")) {
					String code = line.replaceAll("\\W+", CavanString.EMPTY_STRING);

					if (code.length() > 0) {
						Intent intent = new Intent(ACTION_CODE_RECEIVED);
						intent.putExtra("type", "手动输入");
						intent.putExtra("code", code);
						intent.putExtra("shared", false);
						sendBroadcast(intent);
					}
				}
			}
		} else if (preference == mPreferenceRedPacketNotifyTest) {
			if (!CavanAndroid.isNotificationListenerEnabled(this, RedPacketListenerService.class)) {
				PermissionSettingsActivity.startNotificationListenerSettingsActivity(this);
				CavanAndroid.showToastLong(this, "请打开通知读取权限");
			} else if (!CavanAndroid.isAccessibilityServiceEnabled(this, CavanAccessibilityService.class)) {
				PermissionSettingsActivity.startAccessibilitySettingsActivity(this);
				CavanAndroid.showToast(this, "请打开辅助功能");
			} else if (!CavanInputMethod.isDefaultInputMethod(this)) {
				CavanAndroid.showInputMethodPicker(this);
			} else {
				NotificationManager manager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
				if (manager != null) {
					String text = (String) object;

					Builder builder = new Builder(this)
						.setSmallIcon(R.drawable.ic_launcher)
						.setAutoCancel(true)
						.setContentTitle("红包提醒测试")
						.setTicker(text)
						.setContentText(text);

					manager.notify(RedPacketListenerService.NOTIFY_TEST, builder.build());
				}
			}
		} else if (preference == mPreferenceRedPacketNotifyRingtone) {
			updateRingtoneSummary((String) object);
		} else if (preference == mPreferenceWanIp || preference == mPreferenceWanPort || preference == mPreferenceWanShare) {
			if (object instanceof CharSequence) {
				preference.setSummary((CharSequence) object);
			}

			if (mFloatMessageService != null) {
				try {
					mFloatMessageService.updateTcpService();
				} catch (RemoteException e) {
					e.printStackTrace();
				}
			}
		} else if (preference == mPreferenceTcpBridge || preference == mPreferenceTcpBridgeSetting) {
			if (object instanceof CharSequence) {
				preference.setSummary((CharSequence) object);
			}

			if (mFloatMessageService != null) {
				try {
					mFloatMessageService.updateTcpBridge();
				} catch (RemoteException e) {
					e.printStackTrace();
				}
			}
		} else if (preference == mPreferenceAutoCommit || preference == mPreferenceCommitAhead) {
			ListPreference listPreference = (ListPreference) preference;
			int index = listPreference.findIndexOfValue((String) object);
			if (index >= 0) {
				listPreference.setSummary(listPreference.getEntries()[index]);
			}
		} else if (preference == mPreferenceAutoUnlock) {
			if ((boolean) object) {
				CavanAndroid.setLockScreenEnable(this, true);
			}
		}

		return true;
	}
}
