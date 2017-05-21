package com.cavan.cavanmain;

import java.util.Iterator;
import java.util.List;

import android.content.Intent;
import android.os.Message;
import android.view.KeyEvent;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;
import android.widget.EditText;

import com.cavan.android.CavanAccessibility;
import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;
import com.cavan.java.CavanString;

public class CavanAccessibilityAlipay extends CavanAccessibilityBase<RedPacketCode> {

	private static final String CLASS_NAME_EDIT_TEXT = EditText.class.getName();

	private static final long POLL_DELAY = 500;
	private static final long POLL_DELAY_XIUXIU = 2000;
	private static final long UNPACK_OVERTIME = 2000;
	private static final long COMMIT_OVERTIME = 300000;
	private static final long REPEAT_OVERTIME = 10000;
	private static final long POLL_OVERTIME = 60000;
	private static final long CLEAN_OVERTIME = 120000;
	private static final int REPEAT_COUNT = 5;

	private static final int MSG_COMMIT_TIMEOUT = 1;

	private int mCodeCount;
	private RedPacketCode mCode;
	private String mInputtedCode;
	private long mDelay;
	private long mLastAddTime;
	private boolean mXiuXiu;
	private boolean mXiuXiuPending;
	private boolean mAutoOpenAlipay;

	private Runnable mRunnableAlipay = new Runnable() {

		@Override
		public void run() {
			removeCallbacks(this);
			mCodeCount = getRedPacketCodeCount();

			// CavanAndroid.dLog("getRedPacketCodeCount = " + mCodeCount);

			AccessibilityNodeInfo root = getRootInActiveWindow();
			if (root != null && CavanPackageName.ALIPAY.equals(root.getPackageName())) {
				RedPacketCode node = getNextCode();
				postRedPacketCode(node, root);

				if (node == null && mService.startNextPendingActivity()) {
					return;
				}

				if (mCodeCount > 0) {
					startAutoCommitRedPacketCode(POLL_DELAY);
				} else if (mXiuXiu) {
					startAutoCommitRedPacketCode(POLL_DELAY_XIUXIU);
				} else {
					mAutoOpenAlipay = false;
				}
			} else {
				mClassName = CavanString.EMPTY_STRING;

				if (mAutoOpenAlipay && mPackets.size() > 0) {
					RedPacketListenerService.startAlipayActivity(mService);
				}
			}
		}
	};

	public CavanAccessibilityAlipay(CavanAccessibilityService service) {
		super(service);
	}

	private int getRedPacketCodeCount() {
		long time = System.currentTimeMillis() - mLastAddTime;

		CavanAndroid.dLog("getRedPacketCodeCount: time = " + time);

		if (mPackets.size() > 0) {
			if (time > CLEAN_OVERTIME) {
				mPackets.clear();
				return 0;
			}

			return mPackets.size();
		}

		if (time > POLL_OVERTIME) {
			return 0;
		}

		return mService.getRedPacketCodeCount();
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
		if (mCodeCount <= 0 && mXiuXiu == false) {
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

	private boolean isUnpakComplete(AccessibilityNodeInfo root) {
		List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByViewId("com.alipay.android.phone.discovery.envelope:id/list");
		if (nodes != null && nodes.size() > 0) {
			return true;
		}

		nodes = root.findAccessibilityNodeInfosByText("领取成功");
		if (nodes != null && nodes.size() > 0) {
			return true;
		}

		return false;
	}

	private boolean postRedPacketCode(RedPacketCode code, AccessibilityNodeInfo root) {
		CavanAndroid.dLog("xiuxiu = " + mXiuXiu + ", pending = " + mXiuXiuPending);
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
			CavanMessageActivity.setRedPacketCodeReceiveEnabled(true);

			if (code == null) {
				break;
			}

			if (code.maybeInvalid()) {
				if (getWindowTimeConsume() > 1000) {
					setRedPacketCodeInvalid(code);
					startAutoCommitRedPacketCode(0);
					postRedPacketCode(root, null);
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
			if (setRedPacketCodeValid() && mCode.isSendPending()) {
				mService.sendRedPacketCode(mCode);
			}

			if (CavanMessageActivity.isAutoUnpackEnabled(mService)) {
				unpackRedPacket(root);
				startAutoCommitRedPacketCode(500);
			}
			break;

		case "com.alipay.android.phone.discovery.envelope.crowd.CrowdHostActivity":
			if (isUnpakComplete(root)) {
				setRedPacketCodeComplete();
			} else if (setRedPacketCodeValid()) {
				mCode.updateTime();
			}

			performBackAction(root, false);
			mInputtedCode = null;
			break;

		case "com.alipay.mobile.nebulacore.ui.H5Activity":
			if (isCurrentRedPacketCode(mCode)) {
				mCode.setValid();

				if (mCode.isSendPending()) {
					mService.sendRedPacketCode(mCode);
				}

				if (mCodeCount > 0) {
					if (mCode.getRepeatTime() > 0) {
						if (mCode.getRepeatTimeout() > REPEAT_OVERTIME && mCode.getPostCount() > REPEAT_COUNT) {
							setRedPacketCodeComplete();
						} else {
							mCode.updateTime();
						}
					} else {
						mCode.updateRepeatTime(mService);
					}
				}
			}

			long time = getWindowTimeConsume();

			CavanAndroid.dLog("getWindowTimeConsume = " + time);

			if (time < 800) {
				break;
			}

			performBackActionH5(root);
			break;

		case "com.ali.user.mobile.login.ui.AliuserGuideActivity":
		case "com.alipay.mobile.commonui.widget.APNoticePopDialog":
			mAutoOpenAlipay = false;
		case "com.alipay.mobile.security.login.ui.AlipayUserLoginActivity":
			CavanMessageActivity.setRedPacketCodeReceiveEnabled(false);
			if (mCode != null) {
				mCode.setPostPending(false);
			}
			break;

		case "com.alipay.mobile.xiuxiu.ui.RedPacketSettingsActivity":
			if (mXiuXiuPending && startXiuXiu(root)) {
				mXiuXiuPending = false;
			}
			break;

		case "com.alipay.mobile.about.widget.UpdateCommonDialog":
			AccessibilityNodeInfo node = CavanAccessibility.findNodeByViewId(root, "com.alipay.mobile.accountauthbiz:id/update_cancel_tv");
			if (node != null) {
				CavanAccessibility.performClickAndRecycle(node);
				break;
			}

			node = CavanAccessibility.findNodeByText(root, "稍后再说");
			if (node != null) {
				CavanAccessibility.performClickAndRecycle(node);
				break;
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
		mPackets.remove(code);
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

		for (RedPacketCode node : mPackets) {
			if (node.compareTo(code) > 0) {
				code = node;
			}
		}

		return code;
	}

	private boolean gotoRedPacketActivity(AccessibilityNodeInfo root) {
		AccessibilityNodeInfo node = CavanAccessibility.findNodeByText(root, "红包");
		if (node == null) {
			return false;
		}

		AccessibilityNodeInfo parent = node.getParent();
		if (parent == null) {
			node.recycle();
			return false;
		}

		node.performAction(AccessibilityNodeInfo.ACTION_SELECT);
		parent.performAction(AccessibilityNodeInfo.ACTION_CLICK);

		parent.recycle();
		node.recycle();

		return true;
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

		if (code == null) {
			CavanAccessibility.setNodeText(mService, node, null);
			return true;
		}

		mInputtedCode = code.getCode();

		boolean changed = CavanAccessibility.setNodeText(mService, node, code.getCode());

		if (mCode != null) {
			mCode.setPostPending(false);
		}

		code.setPostPending(true);
		mCode = code;

		int msgResId;
		int maxCommitCount = CavanMessageActivity.getAutoCommitCount(mService);

		if (code.isCompleted()) {
			code.updateTime();
			msgResId = R.string.completed_please_manual_commit;
		} else if (maxCommitCount > 0) {
			if (CavanInputMethod.isDefaultInputMethod(mService)) {
				if (code.getCommitCount() < maxCommitCount) {
					long delay = code.getDelay() / 1000;
					if (delay > 0) {
						if (delay != mDelay) {
							mDelay = delay;
							String message = mService.getResources().getString(R.string.auto_commit_after, delay);
							CavanAndroid.showToast(mService, message);
						}

						return false;
					}

					mService.sendBroadcast(new Intent(CavanMessageActivity.ACTION_CODE_COMMIT));
					return true;
				} else {
					msgResId = R.string.commit_too_much_please_manual_commit;
				}
			} else if (CavanMessageActivity.isAutoSwitchImeEnabled(mService)) {
				String ime = mService.getResources().getString(R.string.cavan_input_method);
				mService.setInputMethod(ime, 5);
				return false;
			} else {
				msgResId = R.string.ime_fault_please_manual_commit;
			}
		} else {
			msgResId = R.string.auto_commit_not_enable_please_manual_commit;
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

	private boolean startXiuXiu(AccessibilityNodeInfo root) {
		List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByViewId("com.alipay.mobile.xiuxiu:id/button1");
		if (nodes == null || nodes.isEmpty()) {
			return false;
		}

		for (AccessibilityNodeInfo node : nodes) {
			node.performAction(AccessibilityNodeInfo.ACTION_CLICK);
		}

		return true;
	}

	public void removeCodeAll() {
		mPackets.clear();
	}

	public void removeCode(RedPacketCode code) {
		mPackets.remove(code);
	}

	public void removeCodes() {
		if (CavanInputMethod.isDefaultInputMethod(mService)) {
			int count = 0;
			Iterator<RedPacketCode> iterator = mPackets.iterator();
			while (iterator.hasNext()) {
				RedPacketCode node = iterator.next();
				if (node.isRepeatable()) {
					node.setCompleted();
					iterator.remove();
					count++;
				}
			}

			if (count == 0) {
				iterator = mPackets.iterator();
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
		mLastAddTime = System.currentTimeMillis();

		if (mPackets.contains(code)) {
			return true;
		}

		if (code.isInvalid()) {
			CavanAndroid.dLog("skip invalid code: " + code.getCode());
			return false;
		}

		if (code.isRepeatable()) {
			code.updateRepeatTime(mService);
		}

		mPackets.add(code);

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
		if (mPackets.size() > 0 && isCurrentRedPacketCode(mCode)) {
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
		} else {
			switch (mClassName) {
			case "com.alipay.mobile.xiuxiu.ui.RedPacketSettingsActivity":
				mXiuXiu = true;
				mXiuXiuPending = true;
			case "com.alipay.mobile.nebulacore.ui.H5Activity":
				break;

			case "com.alipay.mobile.framework.app.ui.DialogHelper$APGenericProgressDialog":
				if (mInputtedCode != null) {
					RedPacketCode node = RedPacketCode.getInstence(mInputtedCode, 0, true, false, true);
					mCode = node;
				}
			default:
				mXiuXiu = false;
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
		if (mXiuXiu) {
			return false;
		}

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
