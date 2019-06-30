package com.cavan.cavanmain;

import java.net.InetAddress;
import java.util.HashMap;
import java.util.Map.Entry;
import java.util.Set;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.SharedPreferences;
import android.graphics.Point;
import android.os.Message;
import android.preference.PreferenceManager;
import android.telephony.TelephonyManager;
import android.util.ArraySet;
import android.view.WindowManager;

import com.cavan.accessibility.CavanAccessibilityAlipay;
import com.cavan.accessibility.CavanAccessibilityPackage;
import com.cavan.accessibility.CavanAccessibilityService;
import com.cavan.accessibility.CavanKeyguardActivity;
import com.cavan.accessibility.CavanRedPacket;
import com.cavan.accessibility.CavanUnlockActivity;
import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanThreadedHandler;
import com.cavan.android.TcpExecClient;
import com.cavan.java.CavanJava;

public class CavanMainAccessibilityService extends CavanAccessibilityService {

	public static final String KEY_POSITIONS = "positions";

	public static CavanMainAccessibilityService instance;

	private static final int MSG_SHELL_COMMAND = 1;
	private static final int MSG_INPUT_PROXY = 2;

	public static boolean checkAndOpenSettingsActivity(Context context) {
        if (instance != null) {
            return true;
        }

        PermissionSettingsActivity.startAccessibilitySettingsActivity(context);

        return false;
    }

	private TelephonyManager mTelephonyManager;
	private HashMap<String, Point> mPositions = new HashMap<String, Point>();

	private CavanThreadedHandler mThreadedHandler = new CavanThreadedHandler(CavanMainAccessibilityService.class) {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case MSG_SHELL_COMMAND:
				try {
					InetAddress address = InetAddress.getByAddress(new byte[] { 127, 0, 0, 1 });
					TcpExecClient client = new TcpExecClient(address, 8888);
					client.runCommand((String) msg.obj);
				} catch (Exception e) {
					e.printStackTrace();
				}
				break;

			case MSG_INPUT_PROXY:
				CavanInputProxyConnService.send((String) msg.obj);
				break;
			}
		}
	};

	public CavanMainAccessibilityService() {
		addPackage(new CavanMainAccessibilityMM(this));
		addPackage(new CavanMainAccessibilityQQ(this));
		addPackage(new CavanAccessibilityAlipay(this));
	}

	@Override
	public Class<?> getBroadcastReceiverClass() {
		return CavanBroadcastReceiver.class;
	}

	@Override
	protected String getInputMethodName() {
		return getResources().getString(R.string.cavan_input_method);
	}

	@Override
	public boolean onShowLoginDialog(CavanAccessibilityPackage pkg) {
		final CavanUserInfo[] users = CavanUserInfo.query(getContentResolver(), pkg.getName(), null);
		if (users == null) {
			return false;
		}

		if (users.length < 2) {
			if (users.length > 0) {
				CavanUserInfo info = users[0];
				return login(info.getAccount(), info.getPassword());
			}

			return false;
		}

		String[] items = new String[users.length];
		for (int i = 0; i < items.length; i++) {
			items[i] = users[i].getAccount();
		}

		AlertDialog.Builder builder = new AlertDialog.Builder(this);

		builder.setSingleChoiceItems(items, -1, new OnClickListener() {

			@Override
			public void onClick(DialogInterface dialog, int which) {
				dialog.dismiss();
				CavanUserInfo info = users[which];
				login(info.getAccount(), info.getPassword());
			}
		});

		builder.setCancelable(true);
		AlertDialog dialog = builder.create();
		dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
		dialog.show();

		return true;
	}

	@Override
	public synchronized void showCountDownView(CavanRedPacket packet) {
		FloatMessageService service = FloatMessageService.instance;
		if (service != null) {
			if (packet != null) {
				service.setCountDownTime(packet.getUnpackTime());
			} else {
				service.setCountDownTime(0);
			}
		}
	}

	@Override
	public boolean doShellCommand(String command) {
		mThreadedHandler.obtainMessage(MSG_SHELL_COMMAND, command).sendToTarget();
		return true;
	}

	@Override
	public boolean doInputTap(int x, int y) {
		String command = String.format("TAP %d %d %d %d", x, y, getDisplayWidth(), getDisplayHeight());
		if (CavanInputProxyConnService.send(command)) {
			return true;
		}

		return super.doInputTap(x, y);
	}

	@Override
	public boolean doSaveSubject(String question, String answer) {
		CavanSubjectInfo subject = new CavanSubjectInfo(question, answer);
		return (subject.save(getContentResolver()) != null);
	}

	@Override
	public boolean doRemoveSubject(String question) {
		return (CavanSubjectInfo.delete(getContentResolver(), question) > 0);
	}

	@Override
	public boolean doLoadSubjects(HashMap<String, String> subjects) {
		CavanSubjectInfo[] infos = CavanSubjectInfo.query(getContentResolver());
		if (infos == null) {
			return false;
		}

		for (CavanSubjectInfo info : infos) {
			CavanAndroid.dLog("info = " + info);
			subjects.put(info.getQuestion(), info.getAnswer());
		}

		return true;
	}

	@Override
	public String getPassword(CavanAccessibilityPackage pkg, String username) {
		CavanUserInfo[] users = CavanUserInfo.query(getContentResolver(), pkg.getName(), username);
		if (users != null && users.length == 1) {
			return users[0].getPassword();
		}

		return null;
	}

	@Override
	public void onCreate() {
		super.onCreate();
		instance = this;

		mTelephonyManager = (TelephonyManager) getSystemService(TELEPHONY_SERVICE);

		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
		if (preferences != null) {
			Set<String> positions = preferences.getStringSet(KEY_POSITIONS, null);
			if (positions != null) {
				for (String text : positions) {
					String[] args = text.split("|");
					if (args.length < 3) {
						continue;
					}

					int x = CavanJava.parseInt(args[1]);
					int y = CavanJava.parseInt(args[2]);
					mPositions.put(args[0], new Point(x, y));
				}
			}
		}
	}

	@Override
	public void onDestroy() {
		instance = null;
		super.onDestroy();
	}

	public boolean isGotoIdleEnabled() {
		if (mTelephonyManager != null) {
			int state = mTelephonyManager.getCallState();
			CavanAndroid.dLog("getCallState = " + state);

			if (state != TelephonyManager.CALL_STATE_IDLE) {
				return false;
			}
		}

		return true;
	}

	@Override
	protected void onScreenOff() {
		if (isGotoIdleEnabled()) {
			if (CavanMessageActivity.isDisableKeyguardEnabled(this)) {
				CavanKeyguardActivity.show(this);
			} else {
				CavanAndroid.startLauncher(this);
			}
		}
	}

	@Override
	protected void onCloseSystemDialogs(String reason) {
		CavanAndroid.dLog("onCloseSystemDialogs: " + reason);
	}

	@Override
	public void acquireScreenLock() {
		CavanUnlockActivity.setLockScreenEnable(this, false);
	}

	@Override
	public boolean isInformationGroup(String chatting) {
		if (chatting.contains("皇族") || chatting.contains("线报")) {
			return true;
		}

		if (chatting.contains("讨论组") || chatting.contains("互助群")) {
			return true;
		}

		RedPacketListenerService service = RedPacketListenerService.instance;
		if (instance == null) {
			return false;
		}

		return service.isInformationGroup(chatting);
	}

	private boolean savePositions() {
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
		if (preferences == null) {
			return false;
		}

		ArraySet<String> positions = new ArraySet<String>(mPositions.size());

		for (Entry<String, Point> entry : mPositions.entrySet()) {
			StringBuilder builder = new StringBuilder();
			Point point = entry.getValue();

			builder.append(entry.getKey());
			builder.append('|').append(point.x);
			builder.append('|').append(point.y);

			positions.add(builder.toString());
		}

		preferences.edit().putStringSet("positions", mPositions.keySet());

		return true;
	}

	@Override
	protected boolean onSavePosition(String pkg, Point point) {
		CavanAndroid.dLog(String.format("onSavePosition: %s (%d, %d)", pkg, point.x, point.y));
		mPositions.put(pkg, point);
		return savePositions();
	}

	@Override
	protected Point onReadPosition(String pkg) {
		CavanAndroid.dLog("onReadPosition: " + pkg);
		return mPositions.get(pkg);
	}

	@Override
	protected boolean onRemovePosition(String pkg) {
		if (mPositions.remove(pkg) != null) {
			return savePositions();
		}

		return true;
	}
}
