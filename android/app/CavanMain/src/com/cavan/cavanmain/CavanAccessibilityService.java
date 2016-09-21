package com.cavan.cavanmain;

import java.util.ArrayList;
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
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Parcelable;
import android.os.RemoteException;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;
import com.cavan.java.CavanString;
import com.cavan.java.CavanTimedArray;

public class CavanAccessibilityService extends AccessibilityService {

	private static final long POLL_DELAY = 500;
	private static final long INPUT_OVERTIME = 5000;
	private static final long COMMIT_OVERTIME = 2000;
	private static final long CODE_OVERTIME = 7200000;

	private static final int MSG_INPUT_TIMEOUT = 1;

	private static final String[] PACKAGE_NAMES = {
		CavanPackageName.ALIPAY,
	};

	private long mDelay;
	private long mCommitTime;
	private boolean mAutoStartAlipay;

	private String mClassName = CavanString.EMPTY_STRING;
	private String mPackageName = CavanString.EMPTY_STRING;

	private int mCodeCount;
	private RedPacketCode mCode;
	private CharSequence mInputtedCode;
	private List<RedPacketCode> mCodes = new ArrayList<RedPacketCode>();
	private CavanTimedArray<String> mTimedCodes = new CavanTimedArray<String>(CODE_OVERTIME);

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
					startAutoCommitRedPacketCode(0);
				}
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}
	};

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case MSG_INPUT_TIMEOUT:
				RedPacketCode code = (RedPacketCode) msg.obj;
				code.updateTime();
				break;
			}
		}
	};

	private Runnable mRunnableAlipay = new Runnable() {

		@Override
		public void run() {
			mHandler.removeCallbacks(this);
			mCodeCount = getRedPacketCodeCount();

			CavanAndroid.eLog("getRedPacketCodeCount = " + mCodeCount);

			postRedPacketCode(getNextCode());

			if (mCodeCount > 0) {
				startAutoCommitRedPacketCode(POLL_DELAY);
			} else {
				mAutoStartAlipay = false;
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

				if (mTimedCodes.hasTimedValue(code.getCode())) {
					code.setCompleted();
				} else {
					mTimedCodes.addTimedValue(code.getCode());
				}

				mCodes.add(code);
				startAutoCommitRedPacketCode(0);
				break;

			case MainActivity.ACTION_CODE_REMOVE:
				code = intent.getParcelableExtra("code");
				if (code == null) {
					break;
				}

				mCodes.remove(code);
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
		if (mService != null) {
			try {
				return mService.getCodeCount();
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}

		return mCodes.size();
	}

	public boolean startAutoCommitRedPacketCode(long delayMillis) {
		if (delayMillis > 0) {
			mHandler.postDelayed(mRunnableAlipay, delayMillis);
		} else {
			mAutoStartAlipay = true;
			mHandler.post(mRunnableAlipay);
		}

		return true;
	}

	private void performBackAction(AccessibilityNodeInfo root) {
		if (mCodeCount > 0) {
			List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByViewId("com.alipay.mobile.ui:id/title_bar_back_button");
			if (nodes != null && nodes.size() > 0) {
				for (AccessibilityNodeInfo node : nodes) {
					node.performAction(AccessibilityNodeInfo.ACTION_CLICK);
				}
			} else {
				performGlobalAction(GLOBAL_ACTION_BACK);
			}
		}
	}

	private boolean onAccessibilityEventMM(AccessibilityEvent event) {
		return false;
	}

	private boolean onAccessibilityEventQQ(AccessibilityEvent event) {
		return false;
	}

	private boolean postRedPacketCode(RedPacketCode code) {
		ComponentName info = CavanAndroid.getTopActivityInfo(this);
		if (info != null) {
			mPackageName = info.getPackageName();
		}

		CavanAndroid.eLog("mPackageName = " + mPackageName);

		if (!CavanPackageName.ALIPAY.equals(mPackageName)) {
			if (mAutoStartAlipay && code != null) {
				RedPacketListenerService.startAlipayActivity(this);
			}

			return false;
		}

		AccessibilityNodeInfo root = getRootInActiveWindow();
		if (root == null) {
			return false;
		}

		CavanAndroid.eLog("mClassName = " + mClassName);

		switch (mClassName) {
		case "com.eg.android.AlipayGphone.AlipayLogin":
			if (mCodeCount > 0) {
				gotoRedPacketActivity(root);
			}
			break;

		case "com.alipay.android.phone.discovery.envelope.HomeActivity":
			if (mCodeCount > 0 && code != null) {
				postRedPacketCode(root, code);
			}

			mCommitTime = System.currentTimeMillis();
			break;

		case "com.alipay.mobile.framework.app.ui.DialogHelper$APGenericProgressDialog":
			if (System.currentTimeMillis() - mCommitTime > COMMIT_OVERTIME) {
				if (mCode != null) {
					mCode.subCommitCount();
				}

				performBackAction(root);
			}
			break;

		case "com.alipay.android.phone.discovery.envelope.get.GetRedEnvelopeActivity":
			if (MainActivity.isAutoUnpackEnabled(this)) {
				unpackRedPacket(root);
				startAutoCommitRedPacketCode(500);
			}
			break;

		case "com.alipay.android.phone.discovery.envelope.crowd.CrowdHostActivity":
			setRedPacketCodeComplete();
			performBackAction(root);
			mInputtedCode = null;
			break;

		case "com.alipay.mobile.nebulacore.ui.H5Activity":
			if (mCode != null) {
				mCode.setRepeatable();
				mCode.updateTime();
			}

			performBackAction(root);
			break;

		default:
			if (!mClassName.startsWith("com.alipay.mobile.framework.app.ui.DialogHelper")) {
				performBackAction(root);
			}
		}

		return false;
	}

	private void removeRedPacketCode(RedPacketCode code) {
		mCodes.remove(code);
	}

	private void setRedPacketCodeComplete() {
		if (mCode != null) {
			removeRedPacketCode(mCode);
			mCode = null;
		}
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
		List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByViewId("com.alipay.android.phone.openplatform:id/home_app_view");
		if (nodes == null || nodes.size() <= 0) {
			return false;
		}

		root = nodes.get(0);

		AccessibilityNodeInfo info = findAccessibilityNodeInfoByText(root, "红包");
		if (info == null) {
			return false;
		}

		info.performAction(AccessibilityNodeInfo.ACTION_SELECT);
		root.performAction(AccessibilityNodeInfo.ACTION_CLICK);

		return true;
	}

	private boolean postRedPacketCode(AccessibilityNodeInfo root, RedPacketCode code) {
		List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByViewId("com.alipay.android.phone.discovery.envelope:id/solitaire_edit");
		if (nodes == null || nodes.size() <= 0) {
			return false;
		}

		for (AccessibilityNodeInfo node : nodes) {
			String text = CavanString.fromCharSequence(node.getText());
			if (!text.equals(code.getCode())) {
				if (text.length() > 0) {
					Bundle arguments = new Bundle();
					arguments.putInt(AccessibilityNodeInfo.ACTION_ARGUMENT_SELECTION_START_INT, 0);
					arguments.putInt(AccessibilityNodeInfo.ACTION_ARGUMENT_SELECTION_END_INT, text.length());
					node.performAction(AccessibilityNodeInfo.ACTION_SET_SELECTION, arguments);
				}

				RedPacketListenerService.postRedPacketCode(this, code.getCode());
				node.performAction(AccessibilityNodeInfo.ACTION_PASTE);
			}
		}

		mCode = code;

		long delay = code.getDelay() / 1000;
		if (delay > 0) {
			if (delay != mDelay) {
				mDelay = delay;
				String message = getResources().getString(R.string.text_auto_commit_after, delay);
				CavanAndroid.showToast(this, message);
			}

			return false;
		}

		if (MainActivity.isAutoCommitEnabled(this) && CavanInputMethod.isDefaultInputMethod(this)) {
			if (code.isCompleted() || code.addCommitCount() > 1) {
				return false;
			}

			sendBroadcast(new Intent(MainActivity.ACTION_CODE_COMMIT));

			if (!mHandler.hasMessages(MSG_INPUT_TIMEOUT, code)) {
				Message message = mHandler.obtainMessage(MSG_INPUT_TIMEOUT, code);
				mHandler.sendMessageDelayed(message, INPUT_OVERTIME);
			}

			return true;
		}

		return false;
	}

	private boolean unpackRedPacket(AccessibilityNodeInfo root) {
		List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByViewId("com.alipay.android.phone.discovery.envelope:id/action_chai");
		if (nodes != null && nodes.size() > 0) {
			for (AccessibilityNodeInfo node : nodes) {
				node.performAction(AccessibilityNodeInfo.ACTION_CLICK);
			}

			if (mInputtedCode != null && mService != null) {
				try {
					mService.sendSharedCode(mInputtedCode.toString());
				} catch (RemoteException e) {
					e.printStackTrace();
				}
			}

			return true;
		}

		nodes = root.findAccessibilityNodeInfosByViewId("com.alipay.android.phone.discovery.envelope:id/coupon_action");
		if (nodes != null && nodes.size() > 0) {
			setRedPacketCodeComplete();
			performBackAction(root);
			return true;
		}

		return false;
	}

	private void onWindowStateChanged(AccessibilityEvent event) {
		mClassName = event.getClassName().toString();
		mPackageName = event.getPackageName().toString();

		switch (mPackageName) {
		case CavanPackageName.ALIPAY:
			startAutoCommitRedPacketCode(0);
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

	private void onViewTextChanged(AccessibilityEvent event) {
		switch (event.getPackageName().toString()) {
		case CavanPackageName.ALIPAY:
			AccessibilityNodeInfo source = event.getSource();
			if (source == null) {
				break;
			}

			String id = source.getViewIdResourceName();
			if (id == null) {
				break;
			}

			if (!id.equals("com.alipay.android.phone.discovery.envelope:id/solitaire_edit")) {
				break;
			}

			List<CharSequence> sequences = event.getText();
			if (sequences != null && sequences.size() > 0) {
				mInputtedCode = sequences.get(0);
			}
			break;
		}
	}

	@Override
	public void onAccessibilityEvent(AccessibilityEvent event) {
		switch (event.getEventType()) {
		case AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED:
			onWindowStateChanged(event);
			break;

		case AccessibilityEvent.TYPE_NOTIFICATION_STATE_CHANGED:
			onNotificationStateChanged(event);
			break;

		case AccessibilityEvent.TYPE_VIEW_TEXT_CHANGED:
			onViewTextChanged(event);
			break;
		}
	}

	@Override
	protected void onServiceConnected() {
		AccessibilityServiceInfo info = getServiceInfo();
		info.packageNames = PACKAGE_NAMES;
		info.flags |= AccessibilityServiceInfo.FLAG_REPORT_VIEW_IDS;
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
