package com.cavan.activity;


import java.util.List;

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

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;
import com.cavan.cavanmain.CavanAccessibilityService;
import com.cavan.cavanmain.CavanInputMethod;
import com.cavan.cavanmain.FloatMessageService;
import com.cavan.cavanmain.IFloatMessageService;
import com.cavan.cavanmain.R;
import com.cavan.cavanmain.RedPacketCode;
import com.cavan.cavanmain.RedPacketListenerService;
import com.cavan.java.CavanString;
import com.cavan.resource.EditableMultiSelectListPreference;

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
	public static final String ACTION_SEND_WAN_COMMAN = "cavan.intent.action.ACTION_SEND_WAN_COMMAN";
	public static final String ACTION_UNPACK_QQ = "cavan.intent.action.ACTION_UNPACK_QQ";
	public static final String ACTION_UNPACK_MM = "cavan.intent.action.ACTION_UNPACK_MM";

	public static final String KEY_AUTO_UNLOCK = "auto_unlock";
	public static final String KEY_AUTO_COMMIT = "auto_commit";
	public static final String KEY_AUTO_UNPACK = "auto_unpack";
	public static final String KEY_LISTEN_CLIP = "listen_clip";
	public static final String KEY_LISTEN_CLICK = "listen_click";
	public static final String KEY_FLOAT_TIMER = "float_timer";
	public static final String KEY_LAN_SHARE = "lan_share";
	public static final String KEY_LAN_TEST = "lan_test";
	public static final String KEY_WAN_SHARE = "wan_share";
	public static final String KEY_WAN_RECEIVE = "wan_receive";
	public static final String KEY_WAN_TEST = "wan_test";
	public static final String KEY_WAN_SERVER = "wan_server";
	public static final String KEY_MESSAGE_SHOW = "message_show";
	public static final String KEY_COMMIT_AHEAD = "commit_ahead";
	public static final String KEY_AUTO_OPEN_APP = "auto_open_app";
	public static final String KEY_AUTO_OPEN_ALIPAY = "auto_open_alipay";
	public static final String KEY_INPUT_METHOD_SELECT = "input_method_select";
	public static final String KEY_AUTO_SWITCH_IME = "auto_switch_ime";
	public static final String KEY_PERMISSION_SETTINGS = "permission_settings";
	public static final String KEY_RED_PACKET_CODE_SEND = "red_packet_code_send";
	public static final String KEY_RED_PACKET_CODE_RECOGNIZE = "red_packet_code_recognize";
	public static final String KEY_RED_PACKET_CODE_SPLIT = "red_packet_code_split";
	public static final String KEY_RED_PACKET_CODE_CLEAR = "red_packet_code_clear";
	public static final String KEY_RED_PACKET_NOTIFY_TEST = "red_packet_notify_test";
	public static final String KEY_RED_PACKET_NOTIFY_SETTING = "red_packet_notify_setting";
	public static final String KEY_RED_PACKET_NOTIFY_RINGTONE = "red_packet_notify_ringtone";
	public static final String KEY_TCP_BRIDGE = "tcp_bridge";
	public static final String KEY_TCP_BRIDGE_SETTING = "tcp_bridge_setting";
	public static final String KEY_DISABLE_KEYGUARD = "disable_keyguard";
	public static final String KEY_QQ_AUTO_UNPACK = "qq_auto_unpack";
	public static final String KEY_MM_AUTO_UNPACK = "mm_auto_unpack";
	public static final String KEY_KEYWORD_NOTIFY = "keyword_notify";
	public static final String KEY_AUTO_BACK_DESKTOP = "auto_back_desktop";

	private static boolean sAutoOpenAppEnable = true;
	private static boolean sRedPacketCodeReceiveEnabled = true;

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

	public static boolean isDisableKeyguardEnabled(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_DISABLE_KEYGUARD);
	}

	public static boolean isAutoUnlockEnabled(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_AUTO_UNLOCK);
	}

	public static boolean isAutoUnpackEnabled(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_AUTO_UNPACK);
	}

	public static boolean isAutoBackDesktopEnabled(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_AUTO_BACK_DESKTOP);
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

	public static boolean isAutoSwitchImeEnabled(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_AUTO_SWITCH_IME);
	}

	public static void setRedPacketCodeReceiveEnabled(boolean enable) {
		if (sRedPacketCodeReceiveEnabled != enable) {
			sRedPacketCodeReceiveEnabled = enable;
			CavanAndroid.dLog("sRedPacketCodeReceiveEnabled = " + enable);
		}
	}

	public static boolean isRedPacketCodeReceiveEnabled() {
		return sRedPacketCodeReceiveEnabled;
	}

	public static boolean isWanReceiveEnabled(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_WAN_RECEIVE);
	}

	public static List<String> getWanShareServer(Context context) {
		return EditableMultiSelectListPreference.load(context, KEY_WAN_SERVER);
	}

	public static boolean isTcpBridgeEnabled(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_TCP_BRIDGE);
	}

	public static String getTcpBridgeSetting(Context context) {
		return CavanAndroid.getPreference(context, KEY_TCP_BRIDGE_SETTING, null);
	}

	public static void setAutoOpenAppEnable(boolean enable) {
		sAutoOpenAppEnable = enable;
		CavanAndroid.dLog("sAutoOpenAppEnable = " + enable);
	}

	public static boolean isAutoOpenAppEnabled(Context context) {
		return sAutoOpenAppEnable && CavanAndroid.isPreferenceEnabled(context, KEY_AUTO_OPEN_APP);
	}

	public static boolean isAutoOpenAlipayEnabled(Context context) {
		return sAutoOpenAppEnable && CavanAndroid.isPreferenceEnabled(context, KEY_AUTO_OPEN_ALIPAY);
	}

	public static int getAutoUnpackQQ(Context context) {
		String text = CavanAndroid.getPreference(context, KEY_QQ_AUTO_UNPACK, null);

		try {
			if (text != null) {
				return Integer.parseInt(text);
			}
		} catch (Exception e) {
			e.printStackTrace();
		}

		return -1;
	}

	public static int getAutoUnpackMM(Context context) {
		String text = CavanAndroid.getPreference(context, KEY_MM_AUTO_UNPACK, null);

		try {
			if (text != null) {
				return Integer.parseInt(text);
			}
		} catch (Exception e) {
			e.printStackTrace();
		}

		return -1;
	}

	public static int getCommitAhead(Context context) {
		String text = CavanAndroid.getPreference(context, KEY_COMMIT_AHEAD, null);

		try {
			if (text != null) {
				return Integer.parseInt(text) * 1000;
			}
		} catch (Exception e) {
			e.printStackTrace();
		}

		return 5000;
	}

	public static int getNotifySetting(Context context) {
		String text = CavanAndroid.getPreference(context, KEY_RED_PACKET_NOTIFY_SETTING, null);

		try {
			if (text != null) {
				return Integer.parseInt(text);
			}
		} catch (Exception e) {
			e.printStackTrace();
		}

		return 3;
	}

	public static boolean startSogouOcrActivity(Context context) {
		try {
			Intent intent = new Intent();
			intent.setClassName(CavanPackageName.SOGOU_OCR, "com.sogou.ocrplugin.CameraActivity");
			intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
			context.startActivity(intent);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
	}

	private Preference mPreferenceRedPacketClear;
	private Preference mPreferenceInputMethodSelect;
	private CheckBoxPreference mPreferenceFloatTime;
	private CheckBoxPreference mPreferenceAutoUnlock;
	private Preference mPreferencePermissionSettings;
	private Preference mPreferenceMessageShow;
	private CheckBoxPreference mPreferenceAutoOpenApp;
	private CheckBoxPreference mPreferenceAutoOpenAlipay;
	private EditTextPreference mPreferenceRedPacketCodeSend;
	private EditTextPreference mPreferenceRedPacketCodeRecognize;
	private EditTextPreference mPreferenceRedPacketCodeSplit;
	private EditTextPreference mPreferenceRedPacketNotifyTest;
	private RingtonePreference mPreferenceRedPacketNotifyRingtone;
	private CheckBoxPreference mPreferenceWanShare;
	private CheckBoxPreference mPreferenceWanReceive;
	private EditableMultiSelectListPreference mPreferenceWanServer;
	private Preference mPreferenceLanTest;
	private Preference mPreferenceWanTest;
	private CheckBoxPreference mPreferenceTcpBridge;
	private EditTextPreference mPreferenceTcpBridgeSetting;

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

			CavanAndroid.dLog("action = " + action);

			switch (action) {
			case ACTION_WAN_UPDATED:
				switch (intent.getIntExtra("state", 0)) {
				case R.string.text_wan_connecting:
					mPreferenceWanShare.setSummary(R.string.text_connecting);
					mPreferenceWanServer.setSummary(intent.getStringExtra("summary"));
					break;

				case R.string.text_wan_connected:
					mPreferenceWanShare.setSummary(R.string.text_connected);
					mPreferenceWanServer.setSummary(intent.getStringExtra("summary"));
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
	private ListPreference findListPreference(String key) {
		ListPreference preference = (ListPreference) findPreference(key);
		preference.setSummary(preference.getEntry());
		preference.setOnPreferenceChangeListener(this);
		return preference;
	}

	@SuppressWarnings("deprecation")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.red_packet_settings);

		mPreferenceInputMethodSelect = findPreference(KEY_INPUT_METHOD_SELECT);
		mPreferenceLanTest = findPreference(KEY_LAN_TEST);
		mPreferenceWanTest = findPreference(KEY_WAN_TEST);
		mPreferenceRedPacketClear = findPreference(KEY_RED_PACKET_CODE_CLEAR);
		mPreferenceAutoOpenApp = (CheckBoxPreference) findPreference(KEY_AUTO_OPEN_APP);
		mPreferenceAutoOpenAlipay = (CheckBoxPreference) findPreference(KEY_AUTO_OPEN_ALIPAY);

		mPreferenceAutoUnlock = (CheckBoxPreference) findPreference(KEY_AUTO_UNLOCK);
		mPreferenceAutoUnlock.setOnPreferenceChangeListener(this);

		mPreferenceMessageShow = findPreference(KEY_MESSAGE_SHOW);
		mPreferenceMessageShow.setIntent(MessageActivity.getIntent(this));

		mPreferencePermissionSettings = findPreference(KEY_PERMISSION_SETTINGS);
		mPreferencePermissionSettings.setIntent(PermissionSettingsActivity.getIntent(this));

		mPreferenceFloatTime = (CheckBoxPreference) findPreference(KEY_FLOAT_TIMER);
		mPreferenceFloatTime.setOnPreferenceChangeListener(this);

		mPreferenceWanShare = (CheckBoxPreference) findPreference(KEY_WAN_SHARE);
		mPreferenceWanShare.setOnPreferenceChangeListener(this);

		mPreferenceWanReceive = (CheckBoxPreference) findPreference(KEY_WAN_RECEIVE);

		mPreferenceWanServer = (EditableMultiSelectListPreference) findPreference(KEY_WAN_SERVER);
		mPreferenceWanServer.setOnPreferenceChangeListener(this);

		mPreferenceRedPacketNotifyTest = (EditTextPreference) findPreference(KEY_RED_PACKET_NOTIFY_TEST);
		mPreferenceRedPacketNotifyTest.setPositiveButtonText(R.string.text_test);
		mPreferenceRedPacketNotifyTest.setOnPreferenceChangeListener(this);

		mPreferenceRedPacketCodeSend = (EditTextPreference) findPreference(KEY_RED_PACKET_CODE_SEND);
		mPreferenceRedPacketCodeSend.setPositiveButtonText(R.string.text_send);
		mPreferenceRedPacketCodeSend.setOnPreferenceChangeListener(this);

		mPreferenceRedPacketCodeRecognize = (EditTextPreference) findPreference(KEY_RED_PACKET_CODE_RECOGNIZE);
		mPreferenceRedPacketCodeRecognize.setPositiveButtonText(R.string.text_recognize);
		mPreferenceRedPacketCodeRecognize.setOnPreferenceChangeListener(this);

		mPreferenceRedPacketCodeSplit = (EditTextPreference) findPreference(KEY_RED_PACKET_CODE_SPLIT);
		mPreferenceRedPacketCodeSplit.setPositiveButtonText(R.string.text_split);
		mPreferenceRedPacketCodeSplit.setOnPreferenceChangeListener(this);

		mPreferenceRedPacketNotifyRingtone = (RingtonePreference) findPreference(KEY_RED_PACKET_NOTIFY_RINGTONE);
		String text = mPreferenceRedPacketNotifyRingtone.getPreferenceManager().getSharedPreferences().getString(KEY_RED_PACKET_NOTIFY_RINGTONE, null);
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

		findListPreference(KEY_AUTO_COMMIT);
		findListPreference(KEY_COMMIT_AHEAD);
		findListPreference(KEY_RED_PACKET_NOTIFY_SETTING);
		findListPreference(KEY_QQ_AUTO_UNPACK);
		findListPreference(KEY_MM_AUTO_UNPACK);

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

		super.onDestroy();
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
		if (preference == mPreferenceInputMethodSelect) {
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
			mPreferenceWanReceive.setChecked(true);

			if (mFloatMessageService != null) {
				try {
					mFloatMessageService.sendTcpCommand(FloatMessageService.NET_CMD_TEST);
				} catch (RemoteException e) {
					e.printStackTrace();
				}
			}
		} else if (preference == mPreferenceRedPacketNotifyTest) {
			setRedPacketCodeReceiveEnabled(true);
			setAutoOpenAppEnable(true);

			mPreferenceAutoOpenApp.setChecked(true);
			mPreferenceAutoOpenAlipay.setChecked(true);
			mPreferenceWanReceive.setChecked(true);

			if (!CavanInputMethod.isDefaultInputMethod(this)) {
				CavanAndroid.showInputMethodPicker(this);
			}
		} else if (preference == mPreferenceRedPacketClear) {
			if (mFloatMessageService != null) {
				try {
					mFloatMessageService.removeMessageAll();
				} catch (RemoteException e) {
					e.printStackTrace();
				}
			}

			RedPacketCode.getLastCodes().clear();
			CavanAndroid.showToast(this, R.string.text_already_clear);
		}

		return super.onPreferenceTreeClick(preferenceScreen, preference);
	}

	@Override
	public boolean onPreferenceChange(Preference preference, Object object) {
		if (preference == mPreferenceFloatTime) {
			return setDesktopFloatTimerEnable((boolean) object);
		} else if (preference == mPreferenceRedPacketCodeSend) {
			mPreferenceAutoOpenApp.setChecked(true);
			mPreferenceAutoOpenAlipay.setChecked(true);

			String text = (String) object;
			if (text != null) {
				for (String line : text.split("\n")) {
					String code = RedPacketCode.filtration(line);

					if (code.length() > 0) {
						Intent intent = new Intent(ACTION_CODE_RECEIVED);
						intent.putExtra("type", "手动输入");
						intent.putExtra("code", code);
						intent.putExtra("shared", false);
						sendBroadcast(intent);
					}
				}
			}
		} else if (preference == mPreferenceRedPacketCodeRecognize) {
			mPreferenceAutoOpenApp.setChecked(true);
			mPreferenceAutoOpenAlipay.setChecked(true);

			String text = (String) object;
			if (text != null) {
				Intent intent = new Intent(MainActivity.ACTION_CONTENT_RECEIVED);
				intent.putExtra("desc", "手动输入");
				intent.putExtra("content", CavanString.fromCharSequence(text));
				sendBroadcast(intent);
			}
		} else if (preference == mPreferenceRedPacketCodeSplit) {
			String text = (String) object;
			if (text != null) {
				for (String line : text.split("\n")) {
					String code = line.trim();
					if (code.isEmpty()) {
						continue;
					}

					RedPacketCode.getInstence(code, 0, true, false, false);
				}
			}
		} else if (preference == mPreferenceRedPacketNotifyTest) {
			if (!CavanAndroid.isNotificationListenerEnabled(this, RedPacketListenerService.class)) {
				PermissionSettingsActivity.startNotificationListenerSettingsActivity(this);
				CavanAndroid.showToastLong(this, "请打开通知读取权限");
			} else if (!CavanAndroid.isAccessibilityServiceEnabled(this, CavanAccessibilityService.class)) {
				PermissionSettingsActivity.startAccessibilitySettingsActivity(this);
				CavanAndroid.showToast(this, "请打开辅助功能");
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
		} else if (preference == mPreferenceWanServer || preference == mPreferenceWanShare) {
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
		} else if (preference instanceof ListPreference) {
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
