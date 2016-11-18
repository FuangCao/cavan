package com.cavan.cavanmain;

import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

import android.content.Intent;
import android.os.Bundle;
import android.os.Message;
import android.view.KeyEvent;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;
import android.widget.EditText;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;
import com.cavan.java.CavanString;

public class CavanAccessibilityAlipay extends CavanAccessibilityBase {

	private static final String CLASS_NAME_EDIT_TEXT = EditText.class.getName();

	private static final long POLL_DELAY = 500;
	private static final long UNPACK_OVERTIME = 2000;
	private static final long COMMIT_OVERTIME = 300000;
	private static final long REPEAT_OVERTIME = 20000;

	private static final int MSG_COMMIT_TIMEOUT = 1;

	private int mCodeCount;
	private RedPacketCode mCode;
	private String mInputtedCode;
	private long mDelay;
	private boolean mAutoOpenAlipay;
	private LinkedList<RedPacketCode> mCodes = new LinkedList<RedPacketCode>();

	private Runnable mRunnableAlipay = new Runnable() {

		@Override
		public void run() {
			removeCallbacks(this);
			mCodeCount = getRedPacketCodeCount();

			// CavanAndroid.dLog("getRedPacketCodeCount = " + mCodeCount);

			AccessibilityNodeInfo root = getRootInActiveWindow();
			if (root != null && CavanPackageName.ALIPAY.equals(root.getPackageName())) {
				postRedPacketCode(getNextCode(), root);

				if (mCodeCount > 0) {
					startAutoCommitRedPacketCode(POLL_DELAY);
				} else {
					mAutoOpenAlipay = false;
				}
			} else {
				mClassName = CavanString.EMPTY_STRING;

				if (mAutoOpenAlipay && mCodeCount > 0) {
					RedPacketListenerService.startAlipayActivity(mService);
				}
			}
		}
	};

	public CavanAccessibilityAlipay(CavanAccessibilityService service) {
		super(service);
	}

	private int getRedPacketCodeCount() {
		int count = mService.getRedPacketCodeCount();
		if (count< 0) {
			return mCodes.size();
		}

		return count;
	}

	public boolean startAutoCommitRedPacketCode(long delayMillis) {
		if (delayMillis > 0) {
			postDelayed(mRunnableAlipay, delayMillis);
		} else {
			mAutoOpenAlipay = true;
			post(mRunnableAlipay);
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
				CavanAndroid.dLog("backViewId = " + backViewId);
				node.performAction(AccessibilityNodeInfo.ACTION_CLICK);
			}

			return true;
		}

		if (force) {
			CavanAndroid.dLog("GLOBAL_ACTION_BACK");
			performGlobalAction(CavanAccessibilityService.GLOBAL_ACTION_BACK);
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

	public void startAlipayActivity() {
		mAutoOpenAlipay = true;

		if (isRootActivity()) {
			startAutoCommitRedPacketCode(500);
		} else {
			RedPacketListenerService.startAlipayActivity(mService);
		}
	}

	private boolean postRedPacketCode(RedPacketCode code, AccessibilityNodeInfo root) {
		CavanAndroid.dLog("count = " + mCodeCount + ", code = " + code);

		switch (mClassName) {
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
				if (getWindowTimeConsume() > 1000) {
					setRedPacketCodeInvalid(code);
					startAutoCommitRedPacketCode(0);
				}

				break;
			}

			postRedPacketCode(root, code);

			if (!hasMessages(MSG_COMMIT_TIMEOUT, code)) {
				Message message = obtainMessage(MSG_COMMIT_TIMEOUT, code);
				sendMessageDelayed(message, COMMIT_OVERTIME);
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

			if (MainActivity.isAutoUnpackEnabled(mService)) {
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
					mCode.updateRepeatTime(mService);
				}
			}

			long time = getWindowTimeConsume();

			CavanAndroid.dLog("getWindowTimeConsume = " + time);

			if (time < 800) {
				break;
			}

			performBackActionH5(root);
			break;

		case "com.alipay.mobile.commonui.widget.APNoticePopDialog":
			mAutoOpenAlipay = false;
		case "com.alipay.mobile.security.login.ui.AlipayUserLoginActivity":
			if (mCode != null) {
				mCode.setPostPending(false);
			}
			break;

		default:
			if (mCode != null) {
				mCode.setPostPending(false);
			}

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
		code.setCompleted();
		mCodes.remove(code);
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
		CavanAndroid.dLog("add invalid code: " + code.getCode());
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

		for (RedPacketCode node : mCodes) {
			if (node.compareTo(code) > 0) {
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

		AccessibilityNodeInfo info = CavanAccessibilityService.findAccessibilityNodeInfoByText(root, "红包");
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
			RedPacketListenerService.postRedPacketCode(mService, code.getCode());
			setAccessibilityNodeSelection(node, 0, text.length());
			node.performAction(AccessibilityNodeInfo.ACTION_PASTE);
		}

		if (mCode != null) {
			mCode.setPostPending(false);
		}

		code.setPostPending(true);
		mCode = code;

		int msgResId;
		int maxCommitCount = MainActivity.getAutoCommitCount(mService);

		if (code.isCompleted()) {
			code.updateTime();
			msgResId = R.string.text_completed_please_manual_commit;
		} else if (maxCommitCount > 0) {
			if (CavanInputMethod.isDefaultInputMethod(mService)) {
				if (code.getCommitCount() < maxCommitCount) {
					long delay = code.getDelay() / 1000;
					if (delay > 0) {
						if (delay != mDelay) {
							mDelay = delay;
							String message = mService.getResources().getString(R.string.text_auto_commit_after, delay);
							CavanAndroid.showToast(mService, message);
						}

						return false;
					}

					mService.sendBroadcast(new Intent(MainActivity.ACTION_CODE_COMMIT));
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
			CavanAndroid.showToastLong(mService, msgResId);
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

			if (mInputtedCode != null) {
				RedPacketCode node = RedPacketCode.get(mInputtedCode);
				if (node == null) {
					node = RedPacketCode.getInstence(mInputtedCode, 0, true, false);
					node.setShared();
					mCode = node;
				}

				mService.sendRedPacketCode(mInputtedCode);
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

	public void removeCodeAll() {
		mCodes.clear();
	}

	public void removeCode(RedPacketCode code) {
		mCodes.remove(code);
	}

	public void removeCodes() {
		if (CavanInputMethod.isDefaultInputMethod(mService)) {
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
		}
	}

	public boolean addCode(RedPacketCode code) {
		if (mCodes.contains(code)) {
			return true;
		}

		if (code.isInvalid()) {
			CavanAndroid.dLog("skip invalid code: " + code.getCode());
			return false;
		}

		if (code.isRepeatable()) {
			code.updateRepeatTime(mService);
		}

		mCodes.add(code);

		return true;
	}

	public void setAutoOpenAlipayEnable(boolean enable) {
		mAutoOpenAlipay = enable;
	}

	public void setCommitCount(int count) {
		if (mCode != null) {
			mCode.setCommitCount(count);
		}
	}

	private boolean isRedPacketEditText(AccessibilityNodeInfo node) {
		String id = node.getViewIdResourceName();
		if (id != null) {
			return id.equals("com.alipay.android.phone.discovery.envelope:id/solitaire_edit");
		}

		CharSequence desc = node.getContentDescription();
		if (desc != null) {
			return desc.toString().equals("点击输入口令");
		}

		if ("com.alipay.android.phone.discovery.envelope.HomeActivity".equals(mClassName)) {
			CharSequence className = node.getClassName();
			if (className != null) {
				return className.toString().equals(CLASS_NAME_EDIT_TEXT);
			}
		}

		return false;
	}

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
		}
	}

	@Override
	public String getPackageName() {
		return CavanPackageName.ALIPAY;
	}

	@Override
	public void onWindowStateChanged(AccessibilityEvent event) {
		if (isCurrentRedPacketCode(mCode)) {
			switch (mClassName) {
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
	}

	@Override
	public void onViewTextChanged(AccessibilityEvent event) {
		AccessibilityNodeInfo source = event.getSource();
		if (source != null && isRedPacketEditText(source)) {
			List<CharSequence> sequences = event.getText();
			if (sequences != null && sequences.size() > 0) {
				mInputtedCode = sequences.get(0).toString();
			} else {
				mInputtedCode = null;
			}

			CavanAndroid.dLog("mInputtedCode = " + mInputtedCode);
		}
	}

	@Override
	protected boolean onKeyEvent(KeyEvent event) {
		switch (event.getKeyCode()) {
		case KeyEvent.KEYCODE_VOLUME_UP:
		case KeyEvent.KEYCODE_VOLUME_DOWN:
			if (event.getAction() == KeyEvent.ACTION_DOWN) {
				removeCodes();
			}
			return true;

		case KeyEvent.KEYCODE_BACK:
			if (event.getAction() == KeyEvent.ACTION_DOWN) {
				setCommitCount(0);
			}
			break;
		}

		return false;
	}
}
