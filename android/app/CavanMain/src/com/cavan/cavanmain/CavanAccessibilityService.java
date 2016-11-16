package com.cavan.cavanmain;

import java.util.HashMap;
import java.util.List;

import android.accessibilityservice.AccessibilityService;
import android.accessibilityservice.AccessibilityServiceInfo;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.graphics.Point;
import android.os.IBinder;
import android.os.RemoteException;
import android.view.KeyEvent;
import android.view.WindowManager;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;

public class CavanAccessibilityService extends AccessibilityService {

	private static final String[] PACKAGE_NAMES = {
		CavanPackageName.QQ,
		CavanPackageName.ALIPAY,
		CavanPackageName.SOGOU_IME,
		CavanPackageName.SOGOU_OCR,
	};

	private CavanAccessibilityQQ mAccessibilityQQ = new CavanAccessibilityQQ(this);
	private CavanAccessibilitySogou mAccessibilitySogou = new CavanAccessibilitySogou(this);
	private CavanAccessibilityAlipay mAccessibilityAlipay = new CavanAccessibilityAlipay(this);
	private HashMap<CharSequence, CavanAccessibilityBase> mAccessibilityMap = new HashMap<CharSequence, CavanAccessibilityBase>();

	private Point mDisplaySize = new Point();

	private IFloatMessageService mService;
	private ServiceConnection mConnection = new ServiceConnection() {

		@Override
		public void onServiceDisconnected(ComponentName name) {
			mService = null;
			mAccessibilityAlipay.removeCodeAll();
		}

		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			mService = IFloatMessageService.Stub.asInterface(service);

			try {
				List<String> codes = mService.getCodes();
				if (codes != null && codes.size() > 0) {
					for (String code : codes) {
						RedPacketCode node = RedPacketCode.getInstence(code);
						if (node != null) {
							mAccessibilityAlipay.addCode(node);
						}
					}

					mAccessibilityAlipay.startAlipayActivity();
				}
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}
	};

	private BroadcastReceiver mReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();

			CavanAndroid.dLog("action = " + action);

			switch (action) {
			case MainActivity.ACTION_CODE_TEST:
				if (intent.getStringExtra("code") != null) {
					CavanAndroid.showToast(getApplicationContext(), R.string.text_test_sucess);
				}
				break;

			case MainActivity.ACTION_CODE_ADD:
				RedPacketCode node = RedPacketCode.getInstence(intent);
				if (node != null) {
					mAccessibilityAlipay.addCode(node);
					mAccessibilityAlipay.startAlipayActivity();
				}
				break;

			case MainActivity.ACTION_CODE_REMOVE:
				node = RedPacketCode.getInstence(intent);
				if (node != null) {
					mAccessibilityAlipay.removeCode(node);
				}
				break;

			case Intent.ACTION_CLOSE_SYSTEM_DIALOGS:
				mAccessibilityAlipay.setAutoStartAlipayEnable(false);
				break;
			}
		}
	};

	public CavanAccessibilityService() {
		super();

		mAccessibilityMap.put(CavanPackageName.QQ, mAccessibilityQQ);
		mAccessibilityMap.put(CavanPackageName.ALIPAY, mAccessibilityAlipay);
		mAccessibilityMap.put(CavanPackageName.SOGOU_IME, mAccessibilitySogou);
		mAccessibilityMap.put(CavanPackageName.SOGOU_OCR, mAccessibilitySogou);
	}

	public Point getDisplaySize() {
		return mDisplaySize;
	}

	public int getDisplayWidth() {
		return mDisplaySize.x;
	}

	public int getDisplayHeight() {
		return mDisplaySize.y;
	}

	public void startCheckAutoOpenApp() {
		mAccessibilitySogou.startCheckAutoOpenApp();
	}

	public String getRootPackageName() {
		AccessibilityNodeInfo info = getRootInActiveWindow();
		if (info == null) {
			return null;
		}

		return info.getPackageName().toString();
	}

	public boolean isRootActivity(String pkgName) {
		return pkgName.equals(getRootPackageName());
	}

	public static AccessibilityNodeInfo findAccessibilityNodeInfoByText(AccessibilityNodeInfo root, String text) {
		List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByText(text);
		if (nodes == null) {
			return null;
		}

		for (AccessibilityNodeInfo node : nodes) {
			CharSequence sequence = node.getText();
			if (sequence == null) {
				continue;
			}

			if (text.equals(sequence.toString())) {
				return node;
			}
		}

		return null;
	}

	public int getRedPacketCodeCount() {
		try {
			if (mService != null) {
				return mService.getCodeCount();
			}
		} catch (RemoteException e) {
			e.printStackTrace();
		}

		return -1;
	}

	public boolean sendRedPacketCode(String code) {
		try {
			if (mService != null) {
				mService.sendRedPacketCode(code);
				return true;
			}
		} catch (RemoteException e) {
			e.printStackTrace();
		}

		return false;
	}

	public static void dumpAccessibilityNodeInfo(StringBuilder builder, String prefix, AccessibilityNodeInfo node) {
		if (node == null) {
			return;
		}

		builder.append(prefix);
		builder.append(node);
		builder.append('\n');
		prefix += '\t';

		for (int i = 0, count = node.getChildCount(); i < count; i++) {
			dumpAccessibilityNodeInfo(builder, prefix, node.getChild(i));
		}
	}

	public static String dumpAccessibilityNodeInfo(AccessibilityNodeInfo node) {
		StringBuilder builder = new StringBuilder();
		dumpAccessibilityNodeInfo(builder, "", node);
		return builder.toString();
	}

	@Override
	public void onAccessibilityEvent(AccessibilityEvent event) {
		CavanAccessibilityBase accessibility = mAccessibilityMap.get(event.getPackageName());
		if (accessibility != null) {
			accessibility.dispatchAccessibilityEvent(event);
		}
	}

	@Override
	protected boolean onKeyEvent(KeyEvent event) {
		AccessibilityNodeInfo root = getRootInActiveWindow();
		if (root == null) {
			return false;
		}

		CavanAccessibilityBase accessibility = mAccessibilityMap.get(root.getPackageName());
		if (accessibility == null) {
			return false;
		}

		return accessibility.onKeyEvent(event);
	}

	@Override
	protected void onServiceConnected() {
		AccessibilityServiceInfo info = getServiceInfo();

		info.packageNames = PACKAGE_NAMES;

		info.flags |= AccessibilityServiceInfo.DEFAULT |
				AccessibilityServiceInfo.FLAG_REPORT_VIEW_IDS |
				AccessibilityServiceInfo.FLAG_REQUEST_FILTER_KEY_EVENTS |
				AccessibilityServiceInfo.FLAG_RETRIEVE_INTERACTIVE_WINDOWS;

		info.eventTypes = AccessibilityEvent.TYPE_WINDOW_CONTENT_CHANGED |
				AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED | AccessibilityEvent.TYPE_VIEW_CLICKED |
				AccessibilityEvent.TYPE_VIEW_CLICKED | AccessibilityEvent.TYPE_VIEW_TEXT_CHANGED;

		setServiceInfo(info);

		CavanAndroid.dLog("info = " + getServiceInfo());

		super.onServiceConnected();
	}

	@Override
	public void onInterrupt() {
		CavanAndroid.pLog();
	}

	@Override
	public void onCreate() {
		WindowManager manager = (WindowManager) getSystemService(WINDOW_SERVICE);
		if (manager != null) {
			manager.getDefaultDisplay().getSize(mDisplaySize);
		}

		IntentFilter filter = new IntentFilter();
		filter.addAction(MainActivity.ACTION_CODE_TEST);
		filter.addAction(MainActivity.ACTION_CODE_ADD);
		filter.addAction(MainActivity.ACTION_CODE_REMOVE);
		filter.addAction(Intent.ACTION_CLOSE_SYSTEM_DIALOGS);

		registerReceiver(mReceiver, filter);

		Intent service = FloatMessageService.startService(this);
		bindService(service, mConnection, 0);

		super.onCreate();
	}

	@Override
	public void onDestroy() {
		unbindService(mConnection);
		unregisterReceiver(mReceiver);

		super.onDestroy();
	}
}
