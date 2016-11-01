package com.cavan.cavanmain;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import android.accessibilityservice.AccessibilityService;
import android.accessibilityservice.AccessibilityServiceInfo;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.DialogInterface.OnDismissListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;
import android.widget.CheckBox;
import android.widget.EditText;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;
import com.cavan.java.CavanString;

public class CavanAccessibilityService extends AccessibilityService {

	private static final long POLL_DELAY = 500;
	private static final long UNPACK_OVERTIME = 2000;
	private static final long COMMIT_OVERTIME = 300000;
	private static final long REPEAT_OVERTIME = 20000;

	private static final int MSG_COMMIT_TIMEOUT = 1;
	private static final int MSG_COMMIT_COMPLETE = 2;
	private static final int MSG_CHECK_CONTENT = 3;
	private static final int MSG_CHECK_AUTO_OPEN_APP = 4;

	private static final String[] PACKAGE_NAMES = {
		CavanPackageName.ALIPAY,
		CavanPackageName.QQ,
		CavanPackageName.SOGOU_OCR,
	};

	private long mDelay;
	private boolean mAutoStartAlipay;
	private String mClassNameAlipay = CavanString.EMPTY_STRING;

	private long mWindowStartTime;
	private String mClassName = CavanString.EMPTY_STRING;
	private String mPackageName = CavanString.EMPTY_STRING;

	private int mCodeCount;
	private RedPacketCode mCode;
	private String mInputtedCode;
	private String mLastContent;
	private Dialog mCheckContentDialog;
	private ArrayList<RedPacketCode> mCodes = new ArrayList<RedPacketCode>();

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
				List<String> codes = mService.getCodes();
				if (codes != null && codes.size() > 0) {
					for (String code : codes) {
						RedPacketCode node = RedPacketCode.getInstence(code, false, false);
						if (node != null) {
							mCodes.add(node);
						}
					}

					startAlipayActivity();
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
				if (code.isValid()) {
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

			case MSG_CHECK_CONTENT:
				if (mCheckContentDialog != null && mCheckContentDialog.isShowing()) {
					break;
				}

				String content = (String) msg.obj;

				if (msg.arg1 == 0 && mLastContent != null && mLastContent.equals(content)) {
					break;
				}

				mLastContent = content;

				AlertDialog.Builder builder = new AlertDialog.Builder(CavanAccessibilityService.this, R.style.DialogStyle);

				final View view = View.inflate(CavanAccessibilityService.this, R.layout.red_packet_check, null);

				final EditText editText = (EditText) view.findViewById(R.id.editTextContent);
				editText.setText(content);

				final CheckBox checkBox = (CheckBox) view.findViewById(R.id.checkBoxAsCode);

				builder.setView(view);
				builder.setNegativeButton(android.R.string.cancel, null);
				builder.setPositiveButton(R.string.text_send, new OnClickListener() {

					@Override
					public void onClick(DialogInterface dialog, int which) {
						String text = editText.getText().toString();

						MainActivity.setAutoOpenAppEnable(true);

						if (checkBox != null && checkBox.isChecked()) {
							if (text != null) {
								for (String line : text.split("\n")) {
									String code = line.replaceAll("\\W+", CavanString.EMPTY_STRING);

									if (code.length() > 0) {
										Intent intent = new Intent(MainActivity.ACTION_CODE_RECEIVED);
										intent.putExtra("type", "图片识别");
										intent.putExtra("code", code);
										intent.putExtra("shared", false);
										sendBroadcast(intent);
									}
								}
							}
						} else {
							Intent intent = new Intent(MainActivity.ACTION_CONTENT_RECEIVED);
							intent.putExtra("desc", "图片识别");
							intent.putExtra("content", editText.getText().toString());
							sendBroadcast(intent);
						}
					}
				});

				builder.setOnDismissListener(new OnDismissListener() {

					@Override
					public void onDismiss(DialogInterface dialog) {
						mCheckContentDialog = null;
					}
				});

				mCheckContentDialog = builder.create();
				Window win = mCheckContentDialog.getWindow();

				win.setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
				if (msg.arg1 > 0) {
					win.setGravity(Gravity.CENTER_HORIZONTAL | Gravity.TOP);
					checkBox.setChecked(true);
					mCheckContentDialog.setCanceledOnTouchOutside(true);
				} else {
					mCheckContentDialog.setCancelable(false);
				}

				mCheckContentDialog.show();
				break;

			case MSG_CHECK_AUTO_OPEN_APP:
				removeMessages(MSG_CHECK_AUTO_OPEN_APP);

				String clsName = CavanAndroid.getTopActivityClassName(getApplicationContext());
				CavanAndroid.eLog("ClassName = " + clsName);
				if (clsName == null) {
					sendEmptyMessageDelayed(MSG_CHECK_AUTO_OPEN_APP, 1000);
				} else if (clsName.startsWith("com.sogou.ocrplugin") || clsName.contains("gallery") ||
						clsName.equals("com.tencent.mobileqq.activity.aio.photo.AIOGalleryActivity") ||
						clsName.equals("android.app.Dialog")) {
					MainActivity.setAutoOpenAppEnable(false);
					sendEmptyMessageDelayed(MSG_CHECK_AUTO_OPEN_APP, 2000);
				} else {
					MainActivity.setAutoOpenAppEnable(true);
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

			// CavanAndroid.eLog("getRedPacketCodeCount = " + mCodeCount);

			ComponentName info = CavanAndroid.getTopActivityInfo(CavanAccessibilityService.this);
			if (info != null) {
				mPackageName = info.getPackageName();
			}

			CavanAndroid.eLog("mPackageName = " + mPackageName);

			if (CavanPackageName.ALIPAY.equals(mPackageName)) {
				if (mClassNameAlipay == CavanString.EMPTY_STRING && info != null) {
					String clsName = info.getClassName();
					if (clsName != null) {
						mClassNameAlipay = clsName;
					}
				}

				postRedPacketCode(getNextCode());

				if (mCodeCount > 0) {
					startAutoCommitRedPacketCode(POLL_DELAY);
				} else {
					mAutoStartAlipay = false;
				}
			} else {
				mClassNameAlipay = CavanString.EMPTY_STRING;

				if (mAutoStartAlipay && mCodeCount > 0) {
					RedPacketListenerService.startAlipayActivity(CavanAccessibilityService.this);
				}
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
				if (intent.getStringExtra("code") != null) {
					CavanAndroid.showToast(getApplicationContext(), R.string.text_test_sucess);
				}
				break;

			case MainActivity.ACTION_CODE_ADD:
				RedPacketCode node = RedPacketCode.getInstence(intent);
				if (node == null) {
					break;
				}

				if (node.isInvalid()) {
					CavanAndroid.eLog("skip invalid code: " + node.getCode());
					break;
				}

				if (node.isRepeatable()) {
					node.updateRepeatTime(getApplicationContext());
				}

				mCodes.add(node);
				startAlipayActivity();
				break;

			case MainActivity.ACTION_CODE_REMOVE:
				node = RedPacketCode.getInstence(intent);
				if (node != null) {
					mCodes.remove(node);
				}
				break;

			case Intent.ACTION_CLOSE_SYSTEM_DIALOGS:
				mAutoStartAlipay = false;
				break;
			}
		}
	};

	private void startAlipayActivity() {
		mAutoStartAlipay = true;

		if (CavanAndroid.isTopActivity(this, CavanPackageName.ALIPAY)) {
			startAutoCommitRedPacketCode(500);
		} else {
			RedPacketListenerService.startAlipayActivity(this);
		}
	}

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
				CavanAndroid.eLog("backViewId = " + backViewId);
				node.performAction(AccessibilityNodeInfo.ACTION_CLICK);
			}

			return true;
		}

		if (force) {
			CavanAndroid.eLog("GLOBAL_ACTION_BACK");
			performGlobalAction(GLOBAL_ACTION_BACK);
			return true;
		}

		return false;
	}

	private boolean performBackAction(AccessibilityNodeInfo root, boolean force) {
		return performBackAction(root, "com.alipay.mobile.ui:id/title_bar_back_button", force);
	}

	private boolean performBackActionH5(AccessibilityNodeInfo root) {
		return performBackAction(root, "com.alipay.mobile.nebula:id/h5_tv_nav_back", true);
	}

	private boolean onWindowStateChangedMM(AccessibilityEvent event) {
		return false;
	}

	private boolean onWindowStateChangedQQ(AccessibilityEvent event) {
		if ("com.tencent.mobileqq.activity.aio.photo.AIOGalleryActivity".equals(mClassName)) {
			mHandler.sendEmptyMessage(MSG_CHECK_AUTO_OPEN_APP);
		}

		return false;
	}

	private boolean onWindowStateChangedSogouOcr(AccessibilityEvent event) {
		mHandler.sendEmptyMessage(MSG_CHECK_AUTO_OPEN_APP);

		AccessibilityNodeInfo source = event.getSource();
		if (source == null) {
			return false;
		}

		if ("com.sogou.ocrplugin.OCRResultActivity".equals(mClassName)) {
			List<AccessibilityNodeInfo> nodes = source.findAccessibilityNodeInfosByViewId("com.sohu.inputmethod.sogou:id/result_view");
			if (nodes != null && nodes.size() > 0) {
				CharSequence sequence = nodes.get(0).getText();
				if (sequence != null) {
					Message message = mHandler.obtainMessage(MSG_CHECK_CONTENT, sequence.toString());
					mHandler.sendMessageDelayed(message, 500);
					return true;
				}
			}
		} else {
			mLastContent = null;
		}

		return false;
	}

	private boolean postRedPacketCode(RedPacketCode code) {
		CavanAndroid.eLog("count = " + mCodeCount + ", code = " + code);

		AccessibilityNodeInfo root = getRootInActiveWindow();
		if (root == null) {
			return false;
		}

		switch (mClassNameAlipay) {
		case "com.eg.android.AlipayGphone.AlipayLogin":
			if (mCodeCount > 0) {
				if (mCode != null) {
					mCode.setCommitCount(0);
				}

				gotoRedPacketActivity(root);
			}
			break;

		case "com.alipay.android.phone.discovery.envelope.HomeActivity":
			if (code == null) {
				break;
			}

			if (code.maybeInvalid()) {
				if (getWindowTimeConsume() > 500) {
					setRedPacketCodeInvalid(code);
					startAutoCommitRedPacketCode(0);
				}

				break;
			}

			postRedPacketCode(root, code);

			if (!mHandler.hasMessages(MSG_COMMIT_TIMEOUT, code)) {
				Message message = mHandler.obtainMessage(MSG_COMMIT_TIMEOUT, code);
				mHandler.sendMessageDelayed(message, COMMIT_OVERTIME);
			}
			break;

		case "com.alipay.mobile.framework.app.ui.DialogHelper$APGenericProgressDialog":
			if (getWindowTimeConsume() > UNPACK_OVERTIME && isCurrentRedPacketCode(mCode)) {
				mCode.setCommitCount(0);
				if (!mCode.isValid()) {
					mCode.updateTime();
				}

				performBackAction(root, true);
			}
			break;

		case "com.alipay.android.phone.discovery.envelope.get.GetRedEnvelopeActivity":
			setRedPacketCodeValid();

			if (MainActivity.isAutoUnpackEnabled(this)) {
				unpackRedPacket(root);
				startAutoCommitRedPacketCode(500);
			}
			break;

		case "com.alipay.android.phone.discovery.envelope.crowd.CrowdHostActivity":
			List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByText("领取成功");
			if (nodes != null && nodes.size() > 0) {
				setRedPacketCodeComplete();
			} else if (setRedPacketCodeValid()) {
				mCode.updateTime();
			}

			performBackAction(root, false);
			mInputtedCode = null;
			break;

		case "com.alipay.mobile.nebulacore.ui.H5Activity":
			if (mCodeCount > 0 && isCurrentRedPacketCode(mCode)) {
				if (mCode.getRepeatTime() > 0) {
					if (mCode.getRepeatTimeout() > REPEAT_OVERTIME) {
						setRedPacketCodeComplete();
					} else {
						mCode.updateTime();
					}
				} else {
					mCode.updateRepeatTime(this);
				}
			}

			long time = getWindowTimeConsume();

			CavanAndroid.eLog("getWindowTimeConsume = " + time);

			if (time < 800) {
				break;
			}

			performBackActionH5(root);
			break;

		case "com.alipay.mobile.commonui.widget.APNoticePopDialog":
			mAutoStartAlipay = false;
		case "com.alipay.mobile.security.login.ui.AlipayUserLoginActivity":
			if (mCode != null) {
				mCode.setPostPending(false);
			}
			break;

		default:
			if (mCode != null) {
				mCode.setPostPending(false);
			}

			if (mClassNameAlipay.endsWith("Dialog")) {
				break;
			}

			if (mClassNameAlipay.startsWith("com.alipay.mobile.framework.app.ui.DialogHelper")) {
				break;
			}

			performBackAction(root, true);
		}

		return false;
	}

	private void removeRedPacketCode(RedPacketCode code) {
		code.setCompleted();
		mCodes.remove(code);

		Message message = mHandler.obtainMessage(MSG_COMMIT_COMPLETE, code);
		mHandler.sendMessageDelayed(message, 10000);
	}

	private boolean isCurrentRedPacketCode(RedPacketCode code) {
		if (code == null) {
			return false;
		}

		return mInputtedCode != null && mInputtedCode.equals(code.getCode());
	}

	private boolean setRedPacketCodeComplete() {
		if (isCurrentRedPacketCode(mCode)) {
			removeRedPacketCode(mCode);
			return true;
		}

		if (mCode != null) {
			mCode.updateTime();
		}

		return false;
	}

	private boolean setRedPacketCodeValid() {
		if (isCurrentRedPacketCode(mCode)) {
			mCode.setValid();
			return true;
		}

		return false;
	}

	private void setRedPacketCodeInvalid(RedPacketCode code) {
		CavanAndroid.eLog("add invalid code: " + code.getCode());
		removeRedPacketCode(code);
		code.setInvalid();
	}

	private boolean setRedPacketCodeInvalid() {
		if (isCurrentRedPacketCode(mCode) && mCode.canRemove()) {
			if (mCode.isValid()) {
				return false;
			}

			setRedPacketCodeInvalid(mCode);
			return true;
		}

		return false;
	}

	private RedPacketCode getNextCode() {
		RedPacketCode code = null;
		long time = Long.MAX_VALUE;

		for (RedPacketCode node : mCodes) {
			if (node.getTime() < time) {
				time = node.getTime();
				code = node;
			}
		}

		return code;
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

	private void setAccessibilityNodeSelection(AccessibilityNodeInfo node, int start, int length) {
		if (length > 0) {
			Bundle arguments = new Bundle();
			arguments.putInt(AccessibilityNodeInfo.ACTION_ARGUMENT_SELECTION_START_INT, start);
			arguments.putInt(AccessibilityNodeInfo.ACTION_ARGUMENT_SELECTION_END_INT, start + length);
			node.performAction(AccessibilityNodeInfo.ACTION_SET_SELECTION, arguments);
		} else {
			node.performAction(AccessibilityNodeInfo.ACTION_SELECT);
		}
	}

	private AccessibilityNodeInfo findRedPacketInputNode(AccessibilityNodeInfo root) {
		List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByViewId("com.alipay.android.phone.discovery.envelope:id/solitaire_edit");
		if (nodes != null && nodes.size() > 0) {
			return nodes.get(0);
		}

		return null;
	}

	private boolean postRedPacketCode(AccessibilityNodeInfo root, RedPacketCode code) {
		AccessibilityNodeInfo node = findRedPacketInputNode(root);
		if (node == null) {
			return false;
		}

		String text = CavanString.fromCharSequence(node.getText());
		boolean changed = !text.equals(code.getCode());

		mInputtedCode = code.getCode();

		if (changed) {
			RedPacketListenerService.postRedPacketCode(this, code.getCode());
			setAccessibilityNodeSelection(node, 0, text.length());
			node.performAction(AccessibilityNodeInfo.ACTION_PASTE);
		}

		if (mCode != null) {
			mCode.setPostPending(false);
		}

		code.setPostPending(true);
		mCode = code;

		int msgResId;
		int maxCommitCount = MainActivity.getAutoCommitCount(this);

		if (code.isCompleted()) {
			code.updateTime();
			msgResId = R.string.text_completed_please_manual_commit;
		} else if (maxCommitCount > 0) {
			if (CavanInputMethod.isDefaultInputMethod(this)) {
				if (code.getCommitCount() < maxCommitCount) {
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

		if (changed || getWindowTimeConsume() < 500) {
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
					mService.sendRedPacketCode(mInputtedCode);
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
		mPackageName = event.getPackageName().toString();
		mClassName = event.getClassName().toString();

		CavanAndroid.eLog("mClassName = " + mClassName);

		switch (mPackageName) {
		case CavanPackageName.ALIPAY:
			mClassNameAlipay = mClassName;
			if (isCurrentRedPacketCode(mCode)) {
				switch (mClassNameAlipay) {
				case "com.alipay.mobile.framework.app.ui.DialogHelper$APGenericProgressDialog":
					mCode.setPostComplete();
					break;

				case "com.alipay.android.phone.discovery.envelope.get.GetRedEnvelopeActivity":
				case "com.alipay.android.phone.discovery.envelope.crowd.CrowdHostActivity":
				case "com.alipay.mobile.nebulacore.ui.H5Activity":
					mCode.setValid();
					break;
				}
			}

			startAutoCommitRedPacketCode(500);
			break;

		case CavanPackageName.QQ:
			onWindowStateChangedQQ(event);
			break;

		case CavanPackageName.MM:
			onWindowStateChangedMM(event);
			break;

		case CavanPackageName.SOGOU_OCR:
			onWindowStateChangedSogouOcr(event);
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
				mInputtedCode = sequences.get(0).toString();
			}
		}
	}

	private void onWindowContentChanged(AccessibilityEvent event) {
		final AccessibilityNodeInfo source = event.getSource();
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

		if (MainActivity.isListenClickEnabled(this)) {
			if (id.equals("com.tencent.mobileqq:id/chat_item_content_layout")) {
				String text = CavanString.fromCharSequence(source.getText());

				int lines = CavanString.getLineCount(text);
				if (lines > 0 && lines < 3) {
					mHandler.obtainMessage(MSG_CHECK_CONTENT, 1, 0, text).sendToTarget();
				}

				Intent intent = new Intent(MainActivity.ACTION_CONTENT_RECEIVED);
				intent.putExtra("package", source.getPackageName());
				intent.putExtra("desc", "用户点击");
				intent.putExtra("content", text);
				sendBroadcast(intent);
			}
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
			onViewClicked(event);
			break;

		case AccessibilityEvent.TYPE_VIEW_TEXT_CHANGED:
			onViewTextChanged(event);
			break;
		}
	}

	@Override
	protected boolean onKeyEvent(KeyEvent event) {
		switch (event.getKeyCode()) {
		case KeyEvent.KEYCODE_VOLUME_UP:
		case KeyEvent.KEYCODE_VOLUME_DOWN:
			ComponentName info = CavanAndroid.getTopActivityInfo(this);
			if (info != null && CavanPackageName.ALIPAY.equals(info.getPackageName())) {
				if (event.getAction() == KeyEvent.ACTION_DOWN) {
					if (CavanInputMethod.isDefaultInputMethod(getApplicationContext())) {
						int count = 0;
						Iterator<RedPacketCode> iterator = mCodes.iterator();
						while (iterator.hasNext()) {
							RedPacketCode node = iterator.next();
							if (node.isRepeatable()) {
								node.setCompleted();
								iterator.remove();
								count++;
							}
						}

						if (count == 0) {
							iterator = mCodes.iterator();
							while (iterator.hasNext()) {
								RedPacketCode node = iterator.next();
								if (node.canRemove()) {
									node.setCompleted();
									iterator.remove();
								}
							}
						}
					} else if (mCodeCount > 0) {
						setRedPacketCodeInvalid();
					} else {
						CavanAndroid.showInputMethodPicker(getApplicationContext());
					}
				}

				return true;
			}
			break;

		case KeyEvent.KEYCODE_BACK:
			if (mCode != null) {
				mCode.setCommitCount(0);
			}
			break;
		}

		return super.onKeyEvent(event);
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

		CavanAndroid.eLog("info = " + getServiceInfo());

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
