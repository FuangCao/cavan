package com.cavan.cavanmain;


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

import com.cavan.accessibility.CavanRedPacketAlipay;
import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;
import com.cavan.java.CavanString;
import com.cavan.resource.EditableMultiSelectListPreference;

import java.util.ArrayList;
import java.util.Calendar;
import java.util.Set;

public class CavanMessageActivity extends PreferenceActivity implements OnPreferenceChangeListener {

	public static final String ACTION_BOOT_COMPLETED = "com.cavan.intent.ACTION_BOOT_COMPLETED";
	public static final String ACTION_SERVICE_EXIT = "com.cavan.intent.ACTION_SERVICE_EXIT";
	public static final String ACTION_ON_TIME_NOTIFY = "com.cavan.intent.ACTION_ON_TIME_NOTIFY";
	public static final String ACTION_ON_TIME_MUTE = "com.cavan.intent.ACTION_ON_TIME_MUTE";

	public static final String KEY_AUTO_UNLOCK_LEVEL = "auto_unlock_level";
	public static final String KEY_AUTO_COMMIT = "auto_commit";
	public static final String KEY_AUTO_UNPACK = "auto_unpack";
	public static final String KEY_LISTEN_CLIP = "listen_clip";
	public static final String KEY_LISTEN_CLICK = "listen_click";
	public static final String KEY_FLOAT_TIMER = "float_timer";
	public static final String KEY_ON_TIME_NOTIFY = "on_time_notify";
	public static final String KEY_ON_TIME_SETTING = "on_time_setting";
	public static final String KEY_ON_TIME_MUTE = "on_time_mute";
	public static final String KEY_MUTE_TIME_SETTING = "mute_time_setting";
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
	public static final String KEY_KEYWORD_NOTIFY_ONLY = "keyword_notify_only";
	public static final String KEY_AUTO_BACK_DESKTOP = "auto_back_desktop";
	public static final String KEY_FU_DAI_NOTIFY = "fu_dai_notify";
	public static final String KEY_CLIPBOARD_SHARE = "clipboard_share";
	public static final String KEY_DISABLE_SUSPEND = "disable_suspend";
	public static final String KEY_RED_PACKET_EDIT = "red_packet_edit";
	public static final String KEY_NAME = "name";
	public static final String KEY_PHONE = "phone";
	public static final String KEY_THANKS_NOTIFY = "thanks_notify";
	public static final String KEY_THANKS_SHARE = "thanks_share";
	public static final String KEY_REPEAT_DELAY = "repeat_delay";
	public static final String KEY_NOTIFY_AUTO_CLEAR = "notify_auto_clear";
	public static final String KEY_AUTO_CHECK_PERMISSION = "auto_check_permission";
	public static final String KEY_INFORMATION_NOTIFY = "information_notify";
	public static final String KEY_INFORMATION_GROUPS = "information_groups";

	public static CavanMessageActivity instance;

	private static boolean sAutoOpenAppEnable = true;
	private static boolean sRedPacketCodeReceiveEnabled = true;

	static {
		CavanAndroid.TAG = "CavanMain";
	}

	public static boolean isKeywordNotifyOnly(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_KEYWORD_NOTIFY_ONLY);
	}

	public static boolean isFloatTimerEnabled(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_FLOAT_TIMER);
	}

	public static boolean isOnTimeNotifyEnabled(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_ON_TIME_NOTIFY);
	}

	public static boolean isOnTimeMuteEnabled(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_ON_TIME_MUTE);
	}

	public static String getOnTimeMuteSetting(Context context) {
		return CavanAndroid.getPreference(context, KEY_MUTE_TIME_SETTING, "0");
	}

	public static boolean isOnTimeNotifyEnabledNow(Context context) {
		if (!isOnTimeNotifyEnabled(context)) {
			return false;
		}

		Calendar calendar = Calendar.getInstance();
		int hour = calendar.get(Calendar.HOUR_OF_DAY);

		Set<String> times = CavanAndroid.getPreferenceSet(context, KEY_ON_TIME_SETTING, null);
		if (times == null || times.isEmpty()) {
			return hour > 8;
		}

		int min = (hour * 60 + calendar.get(Calendar.MINUTE) + 10) / 30 * 30;
		CavanAndroid.dLog("isOnTimeNotifyEnabledNow: " + min);

		return times.contains(Integer.toString(min));
	}

	public static int getAutoCommitCount(Context context) {
		return CavanAndroid.getPreferenceInt(context, KEY_AUTO_COMMIT, 1);
	}

	public static boolean isDisableKeyguardEnabled(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_DISABLE_KEYGUARD);
	}

	public static int getAutoUnlockLevel(Context context) {
		return CavanAndroid.getPreferenceInt(context, KEY_AUTO_UNLOCK_LEVEL, 0);
	}

	public static boolean isAutoUnlockEnabled(Context context, int level) {
		CavanAndroid.dLog("level = " + level);
		return (getAutoUnlockLevel(context) >= level);
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

	public static boolean isThanksShareEnabled(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_THANKS_SHARE);
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

	public static int getRepeatDelay(Context context) {
		return CavanAndroid.getPreferenceInt(context, KEY_REPEAT_DELAY, 300);
	}

	public static int getNotifyAutoClear(Context context) {
		return CavanAndroid.getPreferenceInt(context, KEY_NOTIFY_AUTO_CLEAR, 0);
	}

	public static boolean startSogouOcrActivity(Context context) {
		try {
			return CavanAndroid.startActivity(context, CavanPackageName.SOGOU_OCR, "com.sogou.ocrplugin.CameraActivity");
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
	}

	public static boolean isAutoCheckPermissionEnabled(Context context) {
		return CavanAndroid.isPreferenceEnabled(context, KEY_AUTO_CHECK_PERMISSION);
	}

	private Preference mPreferenceRedPacketClear;
	private Preference mPreferenceInputMethodSelect;
	private CheckBoxPreference mPreferenceFloatTime;
	private ListPreference mPreferenceAutoUnlock;
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
	private CheckBoxPreference mPreferenceOnTimeMute;
	private ListPreference mPreferenceMuteTimeSetting;
	private EditableMultiSelectListPreference mPreferenceInformationGroups;

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
		FloatMessageService service = FloatMessageService.instance;

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
		FloatMessageService service = FloatMessageService.instance;

		if (service != null) {
			updateBridgeState(service.getBridgeState());
		}
	}

	private void updateListPreferenceSummary(ListPreference preference, String value) {
		int index = preference.findIndexOfValue(value);
		if (index >= 0) {
			preference.setSummary(preference.getEntries()[index]);
		}
	}

	public boolean addInformationGroups() {
		RedPacketListenerService instance = RedPacketListenerService.instance;
		if (instance == null) {
			return false;
		}

		Set<String> groups = instance.getGroups();
		if (groups.isEmpty()) {
			return false;
		}

		String[] array = new String[groups.size()];
		groups.toArray(array);

		return mPreferenceInformationGroups.addKeywords(false, array);
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

		if (CavanAndroid.SDK_VERSION >= CavanAndroid.SDK_VERSION_40) {
			addPreferencesFromResource(R.xml.red_packet_settings2);
		}

		mPreferenceInputMethodSelect = findPreference(KEY_INPUT_METHOD_SELECT);
		mPreferenceLanTest = findPreference(KEY_LAN_TEST);
		mPreferenceWanTest = findPreference(KEY_WAN_TEST);
		mPreferenceRedPacketClear = findPreference(KEY_RED_PACKET_CODE_CLEAR);
		mPreferenceAutoOpenApp = (CheckBoxPreference) findPreference(KEY_AUTO_OPEN_APP);
		mPreferenceAutoOpenAlipay = (CheckBoxPreference) findPreference(KEY_AUTO_OPEN_ALIPAY);

		mPreferenceAutoUnlock = findListPreference(KEY_AUTO_UNLOCK_LEVEL);
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

		mPreferenceOnTimeMute = (CheckBoxPreference) findPreference(KEY_ON_TIME_MUTE);
		mPreferenceOnTimeMute.setOnPreferenceChangeListener(this);

		mPreferenceMuteTimeSetting = findListPreference(KEY_MUTE_TIME_SETTING);
		mPreferenceMuteTimeSetting.setOnPreferenceChangeListener(this);

		mPreferenceInformationGroups = (EditableMultiSelectListPreference) findPreference(KEY_INFORMATION_GROUPS);
		mPreferenceInformationGroups.setSaveWhenEnabled();
		addInformationGroups();

		findListPreference(KEY_THANKS_NOTIFY);
		findListPreference(KEY_AUTO_COMMIT);
		findListPreference(KEY_COMMIT_AHEAD);
		findListPreference(KEY_RED_PACKET_NOTIFY_SETTING);
		findListPreference(KEY_QQ_AUTO_UNPACK);
		findListPreference(KEY_MM_AUTO_UNPACK);
		findListPreference(KEY_REPEAT_DELAY);
		findListPreference(KEY_NOTIFY_AUTO_CLEAR);

		Intent service = FloatMessageService.buildIntent(this);

		startService(service);
		bindService(service, mFloatMessageConnection, 0);

		instance = this;

		updateWanState();
		updateBridgeState();

		CavanAndroid.checkAndRequestPermissions(this, Manifest.permission.ACCESS_COARSE_LOCATION, Manifest.permission.ACCESS_WIFI_STATE, Manifest.permission.CHANGE_WIFI_STATE);
	}

	@Override
	protected void onDestroy() {
		instance = null;
		unbindService(mFloatMessageConnection);

		super.onDestroy();
	}

	@Override
	public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
		for (int i = 0; i < permissions.length; i++) {
			CavanAndroid.dLog("permissions[" + i + "]: " + permissions[i] + " => " + grantResults[i]);
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

			if (CavanMainInputMethod.instance == null) {
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
		}

		return super.onPreferenceTreeClick(preferenceScreen, preference);
	}

	@Override
	public boolean onPreferenceChange(Preference preference, Object object) {
		if (preference == mPreferenceFloatTime) {
			return setDesktopFloatTimerEnable((Boolean) object);
		} else if (preference == mPreferenceRedPacketCodeSend) {
			mPreferenceAutoOpenApp.setChecked(true);
			mPreferenceAutoOpenAlipay.setChecked(true);

			String text = (String) object;
			if (text != null) {
				for (String line : text.split("\n")) {
					String code = CavanRedPacketAlipay.filtration(line);

					if (code.length() > 0) {
						RedPacketListenerService listener = RedPacketListenerService.instance;
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
				RedPacketListenerService listener = RedPacketListenerService.instance;
				if (listener != null) {
					listener.addRedPacketContent(null, CavanString.fromCharSequence(text), "手动输入");
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

					CavanRedPacketAlipay.get(code, true, true);
				}
			}
		} else if (preference == mPreferenceRedPacketNotifyTest) {
			if (CavanMainApplication.test(this) && CavanMainDeviceAdminReceiver.isAdminActive(this)) {
				NotificationManager manager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
				if (manager != null) {
					String text = (String) object;

					Builder builder = RedPacketNotification.newNotificationBuilder(this, "红包提醒测试", text, null);
					builder.setAutoCancel(true);
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
		} else if (preference == mPreferenceAutoUnlock) {
			CavanAndroid.setLockScreenEnable(this, true);
			updateListPreferenceSummary((ListPreference) preference, (String) object);
		} else if (preference == mPreferenceDisableSuspend) {
			if (mFloatMessageService != null) {
				try {
					mFloatMessageService.setSuspendDisable((Boolean) object);
				} catch (RemoteException e) {
					e.printStackTrace();
				}
			}
		} else if (preference == mPreferenceName || preference == mPreferencePhone) {
			preference.setSummary((CharSequence) object);
		} else if (preference == mPreferenceOnTimeNotify) {
			if ((Boolean) object) {
				CavanBroadcastReceiver.setOnTimeNotifyAlarm(this);
			}
		} else if (preference == mPreferenceOnTimeMute) {
			CavanBroadcastReceiver.setOnTimeMuteAlarm(this, (Boolean) object, mPreferenceMuteTimeSetting.getValue());
		} else if (preference == mPreferenceMuteTimeSetting) {
			String value = (String) object;
			CavanBroadcastReceiver.setOnTimeMuteAlarm(this, mPreferenceOnTimeMute.isChecked(), value);
			updateListPreferenceSummary(mPreferenceMuteTimeSetting, value);
		} else if (preference instanceof ListPreference) {
			updateListPreferenceSummary((ListPreference) preference, (String) object);
		}

		return true;
	}
}
