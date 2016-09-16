package com.cavan.cavanmain;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;

import android.accessibilityservice.AccessibilityService;
import android.accessibilityservice.AccessibilityServiceInfo;
import android.app.Notification;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Handler;
import android.os.IBinder;
import android.os.Parcelable;
import android.os.RemoteException;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;

public class CavanAccessibilityService extends AccessibilityService {

	private static final long POLL_DELAY = 500;
	private static final long COMMIT_OVERTIME = 5000;
	private static final long CODE_OVERTIME = 7200000;

	private static final String[] PACKAGE_NAMES = {
		CavanPackageName.ALIPAY,
	};

	private long mDelay;
	private long mCommitTime;
	private boolean mAutoStartAlipay;

	private RedPacketCode mCode;
	private List<RedPacketCode> mCodes = new ArrayList<RedPacketCode>();
	private HashMap<RedPacketCode, Long> mInvalidCodeMap = new HashMap<RedPacketCode, Long>();

	private IFloatMessageService mService;
	private ServiceConnection mConnection = new ServiceConnection() {

		@Override
		public void onServiceDisconnected(ComponentName name) {
			mService = null;
			mCodes.clear();
		}

		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			mService = IFloatMessageService.Stub.asInterface(service);

			try {
				List<RedPacketCode> codes = mService.getCodes();
				if (codes != null) {
					mCodes = codes;

					if (codes.size() > 0) {
						startAutoCommitRedPacketCode();
					}
				}
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}
	};

	private Handler mHandler = new Handler();
	private Runnable mRunnableAlipay = new Runnable() {

		@Override
		public void run() {
			mHandler.removeCallbacks(this);

			postRedPacketCode(getNextCode());

			if (getRedPacketCodeCount() > 0) {
				mHandler.postDelayed(this, POLL_DELAY);
			}
		}
	};

	private BroadcastReceiver mReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();

			CavanAndroid.eLog("action = " + action);

			switch (action) {
			case MainActivity.ACTION_CODE_ADD:
				RedPacketCode code = intent.getParcelableExtra("code");
				if (code == null) {
					break;
				}

				if (isInvalidCode(code)) {
					String text = getResources().getString(R.string.text_ignore_invalid_code, code.getCode());
					CavanAndroid.showToast(CavanAccessibilityService.this, text);
					break;
				}

				mCodes.add(code);
				startAutoCommitRedPacketCode();
				break;

			case MainActivity.ACTION_CODE_REMOVE:
				try {
					if (mService == null || mService.getCodeCount() > 0) {
						code = intent.getParcelableExtra("code");
						if (code == null) {
							break;
						}

						mCodes.remove(code);
					} else {
						mCodes.clear();
					}
				} catch (RemoteException e) {
					e.printStackTrace();
				}
				break;

			case Intent.ACTION_CLOSE_SYSTEM_DIALOGS:
				mAutoStartAlipay = false;
				break;
			}
		}
	};

	public static AccessibilityNodeInfo findAccessibilityNodeInfoByText(AccessibilityNodeInfo root, String text) {
		List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByText(text);
		if (nodes == null) {
			return null;
		}

		for (AccessibilityNodeInfo node : nodes) {
			if (text.equals(node.getText().toString())) {
				return node;
			}
		}

		return null;
	}

	public int getRedPacketCodeCount() {
		if (mService != null) {
			try {
				return mService.getCodeCount();
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}

		return mCodes.size();
	}

	public void startAutoCommitRedPacketCode() {
		mAutoStartAlipay = true;
		mHandler.post(mRunnableAlipay);
	}

	private boolean onAccessibilityEventMM(AccessibilityEvent event) {
		return false;
	}

	private boolean onAccessibilityEventQQ(AccessibilityEvent event) {
		return false;
	}

	private boolean postRedPacketCode(RedPacketCode code) {
		ComponentName info = CavanAndroid.getTopActivityInfo(this);
		if (info == null || CavanPackageName.ALIPAY.equals(info.getPackageName()) == false) {
			if (mAutoStartAlipay && code != null && code.isRepeatable() == false) {
				RedPacketListenerService.startAlipayActivity(this);
			}

			return false;
		}

		AccessibilityNodeInfo root = getRootInActiveWindow();
		if (root == null) {
			return false;
		}

		switch (info.getClassName()) {
		case "com.eg.android.AlipayGphone.AlipayLogin":
			gotoRedPacketActivity(root);
			break;

		case "com.alipay.android.phone.discovery.envelope.HomeActivity":
			if (code != null && MainActivity.isAutoCommitEnabled(this)) {
				long delay = code.getDelay() / 1000;
				if (delay > 0) {
					if (delay != mDelay) {
						mDelay = delay;
						String text = getResources().getString(R.string.text_auto_commit_after, delay);
						CavanAndroid.showToast(this, text);
					}
					break;
				}

				if (sendRedPacketCode(root, code)) {
					mCommitTime = System.currentTimeMillis();
				}
			}
			break;

		case "com.alipay.mobile.framework.app.ui.DialogHelper$APGenericProgressDialog":
			if (System.currentTimeMillis() - mCommitTime > COMMIT_OVERTIME) {
				if (mCode != null) {
					mCode.subCommitCount();
				}

				performGlobalAction(GLOBAL_ACTION_BACK);
			}
			break;

		case "com.alipay.android.phone.discovery.envelope.get.GetRedEnvelopeActivity":
			if (MainActivity.isAutoUnpackEnabled(this)) {
				unpackRedPacket(root);
			}
			break;

		case "com.alipay.android.phone.discovery.envelope.crowd.CrowdHostActivity":
			setRedPacketCodeComplete();
			performGlobalAction(GLOBAL_ACTION_BACK);
			break;

		case "com.alipay.mobile.nebulacore.ui.H5Activity":
			if (mCode != null) {
				mCode.setRepeatable();
				mCode.updateTime();
				mCode = null;
			}
		default:
			performGlobalAction(GLOBAL_ACTION_BACK);
		}

		return false;
	}

	private void removeRedPacketCode(RedPacketCode code) {
		mCodes.remove(code);
		String text = getResources().getString(R.string.text_complete_code, code.getCode());
		CavanAndroid.showToastLong(this, text);
	}

	private void setRedPacketCodeComplete() {
		if (mCode != null) {
			removeRedPacketCode(mCode);
			mCode = null;
		}
	}

	private void addInvalidCode(RedPacketCode code) {
		long time = System.currentTimeMillis();
		long overtime = time - CODE_OVERTIME;

		removeRedPacketCode(code);

		Iterator<Long> iterator = mInvalidCodeMap.values().iterator();
		while (iterator.hasNext()) {
			if (iterator.next() < overtime) {
				iterator.remove();
			}
		}

		mInvalidCodeMap.put(code, time);
	}

	private boolean isInvalidCode(RedPacketCode code) {
		Long time = mInvalidCodeMap.get(code);
		if (time == null) {
			return false;
		}

		if (System.currentTimeMillis() - time < CODE_OVERTIME) {
			return true;
		}

		mInvalidCodeMap.remove(code);

		return false;
	}

	private RedPacketCode getNextCode() {
		RedPacketCode min = null;
		long time = Long.MAX_VALUE;

		for (RedPacketCode code : mCodes) {
			if (code.getTime() < time) {
				time = code.getTime();
				min = code;
			}
		}

		return min;
	}

	private boolean gotoRedPacketActivity(AccessibilityNodeInfo root) {
		AccessibilityNodeInfo info = findAccessibilityNodeInfoByText(root, "红包");
		if (info == null) {
			return false;
		}

		AccessibilityNodeInfo parent = info.getParent();
		if (parent == null) {
			return false;
		}

		info.performAction(AccessibilityNodeInfo.ACTION_SELECT);
		parent.performAction(AccessibilityNodeInfo.ACTION_CLICK);

		return true;
	}

	private boolean sendRedPacketCode(AccessibilityNodeInfo root, RedPacketCode code) {
		AccessibilityNodeInfo infoInput = null;
		AccessibilityNodeInfo infoCommit = null;

		for (int i = root.getChildCount() - 1; i >= 0; i--) {
			AccessibilityNodeInfo info = root.getChild(i);

			CharSequence desc = info.getContentDescription();
			if (desc == null) {
				continue;
			}

			switch (desc.toString()) {
			case "点击输入口令":
				infoInput = info;
				break;

			case "确定":
				infoCommit = info;
				break;
			}
		}

		if (infoInput == null || infoCommit == null) {
			return false;
		}

		int count = code.addCommitCount();
		if (count > 3) {
			addInvalidCode(code);
			return false;
		} else if ((count & 1) == 0) {
			code.setDelay(COMMIT_OVERTIME);
			return false;
		}

		Intent intent = new Intent(MainActivity.ACTION_CODE_POST);

		if (CavanInputMethod.isDefaultInputMethod(this)) {
			intent.putExtra("code", code);
		} else {
			CharSequence text = infoInput.getText();
			if (text != null && text.length() > 0) {
				mCode = null;
				performGlobalAction(GLOBAL_ACTION_BACK);
				return false;
			}

			RedPacketListenerService.postRedPacketCode(this, code.getCode());
			infoInput.performAction(AccessibilityNodeInfo.ACTION_PASTE);
		}

		mCode = code;
		sendBroadcast(intent);

		return true;
	}

	private void unpackRedPacket(AccessibilityNodeInfo root) {
		AccessibilityNodeInfo info = findAccessibilityNodeInfoByText(root, "拆红包");
		if (info != null) {
			info.performAction(AccessibilityNodeInfo.ACTION_CLICK);
		} else {
			List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByText("看看朋友手气");
			if (nodes != null && nodes.size() > 0) {
				setRedPacketCodeComplete();
				performGlobalAction(GLOBAL_ACTION_BACK);
			}
		}
	}

	private void onWindowStateChanged(AccessibilityEvent event) {
		switch (event.getPackageName().toString()) {
		case CavanPackageName.ALIPAY:
			startAutoCommitRedPacketCode();
			break;

		case CavanPackageName.QQ:
			onAccessibilityEventQQ(event);
			break;

		case CavanPackageName.MM:
			onAccessibilityEventMM(event);
			break;
		}
	}

	private void onNotificationStateChanged(AccessibilityEvent event) {
		Parcelable parcelable = event.getParcelableData();

		CavanAndroid.eLog("parcelable = " + parcelable);

		if (parcelable instanceof Notification) {

		} else {
			for (CharSequence text : event.getText()) {
				CavanAndroid.eLog("text = " + text);
			}
		}
	}

	@Override
	public void onAccessibilityEvent(AccessibilityEvent event) {
		CavanAndroid.eLog("================================================================================");
		CavanAndroid.eLog("event = " + event);

		switch (event.getEventType()) {
		case AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED:
			onWindowStateChanged(event);
			break;

		case AccessibilityEvent.TYPE_NOTIFICATION_STATE_CHANGED:
			onNotificationStateChanged(event);
			break;
		}
	}

	@Override
	protected void onServiceConnected() {
		AccessibilityServiceInfo info = getServiceInfo();
		info.packageNames = PACKAGE_NAMES;
		setServiceInfo(info);

		super.onServiceConnected();
	}

	@Override
	public void onInterrupt() {
		CavanAndroid.pLog();
	}

	@Override
	public void onCreate() {
		IntentFilter filter = new IntentFilter();
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
