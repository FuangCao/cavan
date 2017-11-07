package com.cavan.cavanmain;


import java.util.ArrayList;

import android.Manifest;
import android.app.Notification.Builder;
import android.app.NotificationManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
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
import com.cavan.java.CavanString;
import com.cavan.resource.EditableMultiSelectListPreference;

public class CavanMessageActivity extends PreferenceActivity implements OnPreferenceChangeListener {

	public static final String ACTION_BOOT_COMPLETED = "com.cavan.ACTION_BOOT_COMPLETED";
	public static final String ACTION_SERVICE_EXIT = "com.cavan.ACTION_SERVICE_EXIT";
	public static final String ACTION_CODE_RECEIVED = "com.cavan.ACTION_CODE_RECEIVED";
	public static final String ACTION_ON_TIME_NOTIFY = "com.cavan.ACTION_ON_TIME_NOTIFY";

	public static final String KEY_AUTO_UNLOCK = "auto_unlock";
	public static final String KEY_AUTO_COMMIT = "auto_commit";
	public static final String KEY_AUTO_UNPACK = "auto_unpack";
	public static final String KEY_LISTEN_CLIP = "listen_clip";
	public static final String KEY_LISTEN_CLICK = "listen_click";
	public static final String KEY_FLOAT_TIMER = "float_timer";
	public static final String KEY_ON_TIME_NOTIFY = "on_time_notify";
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
	public static final String KEY_QQ_FILTER = "qq_filter";
	public static final String KEY_MM_FILTER = "mm_filter";
	public static final String KEY_KEYWORD_NOTIFY = "keyword_notify";
	public static final String KEY_AUTO_BACK_DESKTOP = "auto_back_desktop";
	public static final String KEY_FU_DAI_NOTIFY = "fu_dai_notify";
	public static final String KEY_CLIPBOARD_SHARE = "clipboard_share";
	public static final String KEY_DISABLE_SUSPEND = "disable_suspend";
	public static final String KEY_RED_PACKET_EDIT = "red_packet_edit";
	public static final String KEY_NAME = "name";
	public static final String KEY_PHONE = "phone";
	public static final String KEY_THANKS_NOTIFY = "thanks_notify";

	private static CavanMessageActivity sInstance;

	public static CavanMessageActivity getInstance() {
		return sInstance;
	}

	private static boolean sAutoOpenAppEnable = true;
	private static boolean sRedPacketCodeReceiveEnabled = true;

	static {
		CavanAndroid.TAG = "CavanMain";
	}

	public static boolean isFloatTimerEnabled(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_FLOAT_TIMER);
	}

	public static boolean isOnTimeNotifyEnabled(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_ON_TIME_NOTIFY);
	}

	public static int getAutoCommitCount(Context context) {
		return CavanAndroid.getPreferenceInt(context, KEY_AUTO_COMMIT, 1);
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

	public static ArrayList<String> getWanShareServer(Context context) {
		return EditableMultiSelectListPreference.load(context, KEY_WAN_SERVER);
	}

	public static boolean isFuDaiNotifyEnabled(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_FU_DAI_NOTIFY);
	}

	public static boolean isClipboardShareEnabled(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_CLIPBOARD_SHARE);
	}

	public static boolean isDisableSuspendEnabled(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_DISABLE_SUSPEND);
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

	public static int getThanksNotify(Context context) {
		return CavanAndroid.getPreferenceInt(context, KEY_THANKS_NOTIFY, -1);
	}

	public static int getAutoUnpackQQ(Context context) {
		return CavanAndroid.getPreferenceInt(context, KEY_QQ_AUTO_UNPACK, -1);
	}

	public static boolean isQqFilterEnabled(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_QQ_FILTER);
	}

	public static int getAutoUnpackMM(Context context) {
		return CavanAndroid.getPreferenceInt(context, KEY_MM_AUTO_UNPACK, -1);
	}

	public static boolean isMmFilterEnabled(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_MM_FILTER);
	}

	public static String getName(Context context) {
		return CavanAndroid.getPreference(context, KEY_NAME, null);
	}

	public static String getPhone(Context context) {
		return CavanAndroid.getPreference(context, KEY_PHONE, null);
	}

	public static int getCommitAhead(Context context) {
		return CavanAndroid.getPreferenceInt(context, KEY_COMMIT_AHEAD, 0);
	}

	public static int getNotifySetting(Context context) {
		return CavanAndroid.getPreferenceInt(context, KEY_RED_PACKET_NOTIFY_SETTING, 3);
	}

	public static boolean startSogouOcrActivity(Context context) {
		try {
			return CavanAndroid.startActivity(context, CavanPackageName.SOGOU_OCR, "com.sogou.ocrplugin.CameraActivity");
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
	private Preference mPreferenceRedPacketEdit;
	private CheckBoxPreference mPreferenceAutoOpenApp;
	private CheckBoxPreference mPreferenceAutoOpenAlipay;
	private CheckBoxPreference mPreferenceOnTimeNotify;
	private EditTextPreference mPreferenceRedPacketCodeSend;
	private EditTextPreference mPreferenceRedPacketCodeRecognize;
	private EditTextPreference mPreferenceRedPacketCodeSplit;
	private EditTextPreference mPreferenceRedPacketNotifyTest;
	private EditTextPreference mPreferenceName;
	private EditTextPreference mPreferencePhone;
	private RingtonePreference mPreferenceRedPacketNotifyRingtone;
	private CheckBoxPreference mPreferenceWanShare;
	private CheckBoxPreference mPreferenceWanReceive;
	private CheckBoxPreference mPreferenceDisableSuspend;
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

	public void updateWanState(int state, CharSequence summary) {
		switch (state) {
		case R.string.wan_connecting:
			mPreferenceWanShare.setSummary(R.string.connecting);
			mPreferenceWanServer.setSummary(summary);
			break;

		case R.string.wan_connected:
			mPreferenceWanShare.setSummary(R.string.connected);
			mPreferenceWanServer.setSummary(summary);
			break;

		case R.string.wan_disconnected:
			mPreferenceWanShare.setSummary(R.string.disconnected);
			break;
		}
	}

	public void updateWanState() {
		FloatMessageService service = FloatMessageService.getInstance();

		if (service != null) {
			updateWanState(service.getWanState(), service.getWanSummary());
		}
	}

	public void updateBridgeState(int state) {
		switch (state) {
		case R.string.tcp_bridge_running:
			mPreferenceTcpBridge.setSummary(R.string.running);
			break;

		case R.string.tcp_bridge_stopped:
			mPreferenceTcpBridge.setSummary(R.string.stopped);
			break;

		case R.string.tcp_bridge_exit:
			mPreferenceTcpBridge.setSummary(R.string.exit);
			break;
		}
	}

	public void updateBridgeState() {
		FloatMessageService service = FloatMessageService.getInstance();

		if (service != null) {
			updateBridgeState(service.getBridgeState());
		}
	}

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

		mPreferenceRedPacketEdit = findPreference(KEY_RED_PACKET_EDIT);
		mPreferenceRedPacketEdit.setIntent(RedPacketEditActivity.getIntent(this));

		mPreferencePermissionSettings = findPreference(KEY_PERMISSION_SETTINGS);
		mPreferencePermissionSettings.setIntent(PermissionSettingsActivity.getIntent(this));

		mPreferenceFloatTime = (CheckBoxPreference) findPreference(KEY_FLOAT_TIMER);
		mPreferenceFloatTime.setOnPreferenceChangeListener(this);

		mPreferenceDisableSuspend = (CheckBoxPreference) findPreference(KEY_DISABLE_SUSPEND);
		mPreferenceDisableSuspend.setOnPreferenceChangeListener(this);

		mPreferenceWanShare = (CheckBoxPreference) findPreference(KEY_WAN_SHARE);
		mPreferenceWanShare.setOnPreferenceChangeListener(this);

		mPreferenceWanReceive = (CheckBoxPreference) findPreference(KEY_WAN_RECEIVE);

		mPreferenceWanServer = (EditableMultiSelectListPreference) findPreference(KEY_WAN_SERVER);
		mPreferenceWanServer.setOnPreferenceChangeListener(this);

		mPreferenceRedPacketNotifyTest = (EditTextPreference) findPreference(KEY_RED_PACKET_NOTIFY_TEST);
		mPreferenceRedPacketNotifyTest.setPositiveButtonText(R.string.test);
		mPreferenceRedPacketNotifyTest.setOnPreferenceChangeListener(this);

		mPreferenceName = (EditTextPreference) findPreference(KEY_NAME);
		mPreferenceName.setSummary(mPreferenceName.getText());
		mPreferenceName.setOnPreferenceChangeListener(this);

		mPreferencePhone = (EditTextPreference) findPreference(KEY_PHONE);
		mPreferencePhone.setSummary(mPreferencePhone.getText());
		mPreferencePhone.setOnPreferenceChangeListener(this);

		mPreferenceRedPacketCodeSend = (EditTextPreference) findPreference(KEY_RED_PACKET_CODE_SEND);
		mPreferenceRedPacketCodeSend.setPositiveButtonText(R.string.send);
		mPreferenceRedPacketCodeSend.setOnPreferenceChangeListener(this);

		mPreferenceRedPacketCodeRecognize = (EditTextPreference) findPreference(KEY_RED_PACKET_CODE_RECOGNIZE);
		mPreferenceRedPacketCodeRecognize.setPositiveButtonText(R.string.recognize);
		mPreferenceRedPacketCodeRecognize.setOnPreferenceChangeListener(this);

		mPreferenceRedPacketCodeSplit = (EditTextPreference) findPreference(KEY_RED_PACKET_CODE_SPLIT);
		mPreferenceRedPacketCodeSplit.setPositiveButtonText(R.string.split);
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

		mPreferenceOnTimeNotify = (CheckBoxPreference) findPreference(KEY_ON_TIME_NOTIFY);
		mPreferenceOnTimeNotify.setOnPreferenceChangeListener(this);

		findListPreference(KEY_THANKS_NOTIFY);
		findListPreference(KEY_AUTO_COMMIT);
		findListPreference(KEY_COMMIT_AHEAD);
		findListPreference(KEY_RED_PACKET_NOTIFY_SETTING);
		findListPreference(KEY_QQ_AUTO_UNPACK);
		findListPreference(KEY_MM_AUTO_UNPACK);

		Intent service = FloatMessageService.buildIntent(this);

		startService(service);
		bindService(service, mFloatMessageConnection, 0);

		sInstance = this;

		updateWanState();
		updateBridgeState();

		CavanAndroid.checkAndRequestPermissions(this, Manifest.permission.ACCESS_COARSE_LOCATION, Manifest.permission.ACCESS_COARSE_LOCATION);
	}

	@Override
	protected void onDestroy() {
		sInstance = null;
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
			CavanAndroid.showToast(this, R.string.already_clear);
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
						RedPacketListenerService listener = RedPacketListenerService.getInstance();
						if (listener != null) {
							listener.addRedPacketCode(code, "手动输入", false);
						}
					}
				}
			}
		} else if (preference == mPreferenceRedPacketCodeRecognize) {
			mPreferenceAutoOpenApp.setChecked(true);
			mPreferenceAutoOpenAlipay.setChecked(true);

			String text = (String) object;
			if (text != null) {
				RedPacketListenerService listener = RedPacketListenerService.getInstance();
				if (listener != null) {
					listener.addRedPacketContent(null, CavanString.fromCharSequence(text), "手动输入", false, true, 0);
				}
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
				FloatMessageService.showToast("请打开通知读取权限");
			} else if (!CavanAccessibilityService.checkAndOpenSettingsActivity(this)) {
				FloatMessageService.showToast("请打开辅助功能");
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
		} else if (preference == mPreferenceDisableSuspend) {
			if (mFloatMessageService != null) {
				try {
					mFloatMessageService.setSuspendDisable((boolean) object);
				} catch (RemoteException e) {
					e.printStackTrace();
				}
			}
		} else if (preference == mPreferenceName || preference == mPreferencePhone) {
			preference.setSummary((CharSequence) object);
		} else if (preference == mPreferenceOnTimeNotify) {
			if ((boolean) object) {
				CavanBroadcastReceiver.setOnTimeNotifyAlarm(this);
			}
		}

		return true;
	}
}
