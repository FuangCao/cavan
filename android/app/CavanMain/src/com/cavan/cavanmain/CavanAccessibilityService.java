package com.cavan.cavanmain;

import java.util.ArrayList;
import java.util.List;

import android.accessibilityservice.AccessibilityService;
import android.accessibilityservice.AccessibilityServiceInfo;
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
import android.os.RemoteException;
import android.view.KeyEvent;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;
import com.cavan.java.CavanString;
import com.cavan.java.CavanTimedArray;

public class CavanAccessibilityService extends AccessibilityService {

	private static final long POLL_DELAY = 500;
	private static final long INPUT_OVERTIME = 5000;
	private static final long UNPACK_OVERTIME = 2000;
	private static final long COMMIT_OVERTIME = 300000;
	private static final long CODE_OVERTIME = 7200000;

	private static final int MSG_COMMIT_TIMEOUT = 1;
	private static final int MSG_COMMIT_COMPLETE = 2;

	private static final String[] PACKAGE_NAMES = {
		CavanPackageName.ALIPAY,
		CavanPackageName.QQ,
	};

	private long mDelay;
	private long mCommitTime;
	private boolean mAutoStartAlipay;

	private long mWindowStartTime;
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
			case MSG_COMMIT_TIMEOUT:
				RedPacketCode code = (RedPacketCode) msg.obj;
				if (code.isRepeatable()) {
					break;
				}

				removeRedPacketCode((RedPacketCode) msg.obj);
				break;

			case MSG_COMMIT_COMPLETE:
				try {
					if (mService != null && mService.getCodeCount() > 0) {
						Message message = obtainMessage(msg.what, msg.obj);
						sendMessageDelayed(message, 5000);
					} else {
						code = (RedPacketCode) msg.obj;
						String text = getResources().getString(R.string.text_complete_code, code.getCode());
						CavanAndroid.showToast(CavanAccessibilityService.this, text);
					}
				} catch (Exception e) {
					e.printStackTrace();
				}
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
			case MainActivity.ACTION_CODE_TEST:
				RedPacketCode code = intent.getParcelableExtra("code");
				if (code != null) {
					CavanAndroid.showToast(getApplicationContext(), R.string.text_test_sucess);
				}
				break;

			case MainActivity.ACTION_CODE_ADD:
				code = intent.getParcelableExtra("code");
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

	private long getWindowTimeConsume() {
		return System.currentTimeMillis() - mWindowStartTime;
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

	private boolean performBackAction(AccessibilityNodeInfo root, String backViewId, boolean force) {
		if (mCodeCount <= 0) {
			return false;
		}

		List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByViewId(backViewId);
		if (nodes != null && nodes.size() > 0) {
			for (AccessibilityNodeInfo node : nodes) {
				node.performAction(AccessibilityNodeInfo.ACTION_CLICK);
			}

			return true;
		}

		if (force) {
			performGlobalAction(GLOBAL_ACTION_BACK);
			return true;
		}

		return false;
	}

	private boolean performBackAction(AccessibilityNodeInfo root, boolean force) {
		return performBackAction(root, "com.alipay.mobile.ui:id/title_bar_back_button", force);
	}

	private boolean performBackActionH5(AccessibilityNodeInfo root) {
		return performBackAction(root, "com.alipay.mobile.nebula:id/h5_tv_nav_back", false);
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
				if (code.getTimeout() > INPUT_OVERTIME) {
					if (!mHandler.hasMessages(MSG_COMMIT_TIMEOUT, code)) {
						Message message = mHandler.obtainMessage(MSG_COMMIT_TIMEOUT, code);
						mHandler.sendMessageDelayed(message, COMMIT_OVERTIME);
					}

					code.updateTime();
				}
			}

			mCommitTime = System.currentTimeMillis();
			break;

		case "com.alipay.mobile.framework.app.ui.DialogHelper$APGenericProgressDialog":
			if (System.currentTimeMillis() - mCommitTime > UNPACK_OVERTIME) {
				if (mCode != null) {
					mCode.subCommitCount();
				}

				performBackAction(root, true);
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
			performBackAction(root, false);
			mInputtedCode = null;
			break;

		case "com.alipay.mobile.nebulacore.ui.H5Activity":
			if (mCode != null) {
				if (mCode.isRepeatable()) {
					mCode.updateTime();
				} else {
					mCode.alignTime(300000, MainActivity.getCommitAhead(this) * 1000);
					mCode.setRepeatable();
				}
			}

			long time = getWindowTimeConsume();

			CavanAndroid.eLog("getWindowTimeConsume = " + time);

			if (time > 800) {
				performBackActionH5(root);
			}
			break;

		case "com.alipay.mobile.commonui.widget.APNoticePopDialog":
			mAutoStartAlipay = false;
		case "com.alipay.mobile.security.login.ui.AlipayUserLoginActivity":
			break;

		default:
			if (mClassName.endsWith("Dialog")) {
				break;
			}

			if (mClassName.startsWith("com.alipay.mobile.framework.app.ui.DialogHelper")) {
				break;
			}

			performBackAction(root, true);
		}

		return false;
	}

	private void removeRedPacketCode(RedPacketCode code) {
		mCodes.remove(code);

		Message message = mHandler.obtainMessage(MSG_COMMIT_COMPLETE, code);
		mHandler.sendMessageDelayed(message, 10000);
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

		AccessibilityNodeInfo node = nodes.get(0);
		String text = CavanString.fromCharSequence(node.getText());
		boolean codeNotMatch = !text.equals(code.getCode());

		if (codeNotMatch) {
			if (text.length() > 0) {
				Bundle arguments = new Bundle();
				arguments.putInt(AccessibilityNodeInfo.ACTION_ARGUMENT_SELECTION_START_INT, 0);
				arguments.putInt(AccessibilityNodeInfo.ACTION_ARGUMENT_SELECTION_END_INT, text.length());
				node.performAction(AccessibilityNodeInfo.ACTION_SET_SELECTION, arguments);
			}

			RedPacketListenerService.postRedPacketCode(this, code.getCode());
			node.performAction(AccessibilityNodeInfo.ACTION_PASTE);
		}

		mCode = code;

		int msgResId;
		int maxCommitCount = MainActivity.getAutoCommitCount(this);

		if (maxCommitCount > 0) {
			if (CavanInputMethod.isDefaultInputMethod(this)) {
				if (code.isCompleted()) {
					msgResId = R.string.text_completed_please_manual_commit;
				} else if (code.getCommitCount() < maxCommitCount) {
					long delay = code.getDelay() / 1000;
					if (delay > 0) {
						if (delay != mDelay) {
							mDelay = delay;
							String message = getResources().getString(R.string.text_auto_commit_after, delay);
							CavanAndroid.showToast(this, message);
						}

						return false;
					}

					sendBroadcast(new Intent(MainActivity.ACTION_CODE_COMMIT));
					code.addCommitCount();
					return true;
				} else {
					msgResId = R.string.text_commit_too_much_please_manual_commit;
				}
			} else {
				msgResId = R.string.text_ime_fault_please_manual_commit;
			}
		} else {
			msgResId = R.string.text_auto_commit_not_enable_please_manual_commit;
		}

		if (codeNotMatch) {
			CavanAndroid.showToastLong(this, msgResId);
		}

		return false;
	}

	private boolean unpackRedPacket(AccessibilityNodeInfo root) {
		CavanAndroid.cancelToast();

		List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByViewId("com.alipay.android.phone.discovery.envelope:id/action_chai");
		if (nodes != null && nodes.size() > 0) {
			for (AccessibilityNodeInfo node : nodes) {
				node.performAction(AccessibilityNodeInfo.ACTION_CLICK);
			}

			if (mInputtedCode != null && mService != null) {
				try {
					mService.sendRedPacketCode(mInputtedCode.toString());
				} catch (RemoteException e) {
					e.printStackTrace();
				}
			}

			return true;
		}

		nodes = root.findAccessibilityNodeInfosByViewId("com.alipay.android.phone.discovery.envelope:id/coupon_action");
		if (nodes != null && nodes.size() > 0) {
			setRedPacketCodeComplete();
			performBackAction(root, false);
			return true;
		}

		return false;
	}

	private void onWindowStateChanged(AccessibilityEvent event) {
		mWindowStartTime = System.currentTimeMillis();
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

	private void onViewTextChanged(AccessibilityEvent event) {
		if (CavanPackageName.ALIPAY.equals(event.getPackageName().toString())) {
			AccessibilityNodeInfo source = event.getSource();
			if (source == null) {
				return;
			}

			String id = source.getViewIdResourceName();
			if (id == null || id.equals("com.alipay.android.phone.discovery.envelope:id/solitaire_edit") == false) {
				return;
			}

			List<CharSequence> sequences = event.getText();
			if (sequences != null && sequences.size() > 0) {
				mInputtedCode = sequences.get(0);
			}
		}
	}

	private void onWindowContentChanged(AccessibilityEvent event) {
		AccessibilityNodeInfo source = event.getSource();
		if (source == null) {
			return;
		}

		String id = source.getViewIdResourceName();
		if (id == null) {
			return;
		}

		if (id.equals("com.tencent.mobileqq:id/msgbox")) {
			CharSequence sequence = source.getText();
			if (sequence != null) {
				String text = sequence.toString();
				if (text.indexOf("[QQ红包]") >= 0) {
					source.performAction(AccessibilityNodeInfo.ACTION_CLICK);
				}

				Intent intent = new Intent(MainActivity.ACTION_CONTENT_RECEIVED);
				intent.putExtra("package", source.getPackageName());
				intent.putExtra("desc", "QQ消息盒子");
				intent.putExtra("content", text);
				intent.putExtra("hasPrefix", true);
				sendBroadcast(intent);
			}
		}
	}

	private void onViewClicked(AccessibilityEvent event) {
		AccessibilityNodeInfo source = event.getSource();
		if (source == null) {
			return;
		}

		String id = source.getViewIdResourceName();
		if (id == null) {
			AccessibilityNodeInfo parent = source.getParent();
			if (parent == null) {
				return;
			}

			id = parent.getViewIdResourceName();
			if (id == null) {
				return;
			}
		}

		if (id.equals("com.tencent.mobileqq:id/chat_item_content_layout")) {
			Intent intent = new Intent(MainActivity.ACTION_CONTENT_RECEIVED);
			intent.putExtra("package", source.getPackageName());
			intent.putExtra("desc", "用户点击");
			intent.putExtra("content", CavanString.fromCharSequence(source.getText()));
			sendBroadcast(intent);
		}
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
		/* CavanAndroid.eLog("=============================================================================");
		CavanAndroid.eLog("event = " + event);
		CavanAndroid.eLog(dumpAccessibilityNodeInfo(event.getSource())); */

		switch (event.getEventType()) {
		case AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED:
			onWindowStateChanged(event);
			break;

		case AccessibilityEvent.TYPE_WINDOW_CONTENT_CHANGED:
			onWindowContentChanged(event);
			break;

		case AccessibilityEvent.TYPE_VIEW_CLICKED:
			if (MainActivity.isListenClickEnabled(this)) {
				onViewClicked(event);
			}
			break;

		case AccessibilityEvent.TYPE_VIEW_TEXT_CHANGED:
			onViewTextChanged(event);
			break;
		}
	}

	@Override
	protected boolean onKeyEvent(KeyEvent event) {
		CavanAndroid.pLog("event = " + event);
		return super.onKeyEvent(event);
	}

	@Override
	protected void onServiceConnected() {
		AccessibilityServiceInfo info = getServiceInfo();

		info.packageNames = PACKAGE_NAMES;

		info.flags |= AccessibilityServiceInfo.FLAG_REPORT_VIEW_IDS |
				AccessibilityServiceInfo.FLAG_REQUEST_FILTER_KEY_EVENTS |
				AccessibilityServiceInfo.FLAG_RETRIEVE_INTERACTIVE_WINDOWS;

		info.eventTypes = AccessibilityEvent.TYPE_WINDOW_CONTENT_CHANGED |
				AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED | AccessibilityEvent.TYPE_VIEW_CLICKED |
				AccessibilityEvent.TYPE_VIEW_CLICKED | AccessibilityEvent.TYPE_VIEW_TEXT_CHANGED;

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
