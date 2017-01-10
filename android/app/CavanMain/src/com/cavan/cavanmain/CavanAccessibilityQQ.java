package com.cavan.cavanmain;

import java.util.LinkedList;

import android.accessibilityservice.AccessibilityService;
import android.content.Intent;
import android.graphics.Rect;
import android.os.Build;
import android.os.Message;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanAccessibility;
import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;

public class CavanAccessibilityQQ extends CavanAccessibilityBase {

	private static final String RED_PACKET_NAME = "QQ红包";

	private static final int POLL_DELAY = 200;
	private static final int POLL_DELAY_BACK = 2000;
	private static final int MAX_RETRY_COUNT = 3;

	private static final int MSG_ADD_PACKET = 1;
	private static final int MSG_UNPACK_TIMEOUT = 2;

	private int mChildIndex;
	private int mRetryCount;
	private boolean mUnpackPending;
	private String mMessageBoxText;

	private LinkedList<String> mPackets = new LinkedList<String>();
	private Runnable mRunnablePoll = new Runnable() {

		@Override
		public void run() {
			removeCallbacks(this);

			long delay = doFindAndUnpack();
			if (delay > 0) {
				startAutoUnpack(delay);
			}
		}
	};

	public CavanAccessibilityQQ(CavanAccessibilityService service) {
		super(service);
	}

	public boolean isMessageBoxNode(AccessibilityNodeInfo node) {
		if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
			String id = node.getViewIdResourceName();
			return "com.tencent.mobileqq:id/msgbox".equals(id);
		}

		if (node.isMultiLine()) {
			return false;
		}

		if (CavanAccessibility.isTextView(node)) {
			Rect bounds = new Rect();

			node.getBoundsInScreen(bounds);
			if (bounds.width() < mService.getDisplayWidth()) {
				return false;
			}

			node.getBoundsInParent(bounds);
			if (bounds.top != 0) {
				return false;
			}

			return true;
		}

		return false;
	}

	private void resetRetryCount() {
		mChildIndex = 0;
		mRetryCount = 0;
	}

	private void startAutoUnpack(long delay) {
		postDelayed(mRunnablePoll, delay);
	}

	private void startAutoUnpack() {
		startAutoUnpack(POLL_DELAY);
	}

	private void setUnpackPending(boolean pending) {
		removeMessages(MSG_UNPACK_TIMEOUT);

		if (pending) {
			mUnpackPending = true;
			sendEmptyMessageDelayed(MSG_UNPACK_TIMEOUT, POLL_DELAY_BACK);
		} else {
			mUnpackPending = false;
			performGlobalAction(AccessibilityService.GLOBAL_ACTION_BACK);
		}
	}

	public void addRedPacket(String name) {
		int delay = MainActivity.getAutoUnpackQQ(mService);
		if (delay < 0) {
			return;
		}

		Message message = obtainMessage(MSG_ADD_PACKET, name);
		sendMessageDelayed(message, delay * 1000);
	}

	private boolean doAutoUnpack(AccessibilityNodeInfo root) {
		for (AccessibilityNodeInfo node : root.findAccessibilityNodeInfosByText(RED_PACKET_NAME)) {
			AccessibilityNodeInfo parent = node.getParent();
			if (parent == null) {
				continue;
			}

			node.recycle();

			if (parent.getChildCount() < 3) {
				parent.recycle();
				continue;
			}

			String text = CavanAccessibility.getChildNodeText(parent, 2);
			CavanAndroid.dLog("name = " + text);

			if (!RED_PACKET_NAME.equals(text)) {
				parent.recycle();
				continue;
			}

			String message = CavanAccessibility.getChildNodeText(parent, 0);
			CavanAndroid.dLog("message = " + message);

			if (message == null || message.contains("测") || message.contains("挂")) {
				parent.recycle();
				continue;
			}

			text = CavanAccessibility.getChildNodeText(parent, 1);
			CavanAndroid.dLog("state = " + text);

			if (text == null) {
				parent.recycle();
				continue;
			}

			switch (text) {
			case "点击拆开":
				setUnpackPending(true);
				parent.performAction(AccessibilityNodeInfo.ACTION_CLICK);
				parent.recycle();
				return true;

			case "口令红包":
				AccessibilityNodeInfo inputBar = CavanAccessibility.findNodeByViewId(root, "com.tencent.mobileqq:id/inputBar");
				if (inputBar == null || inputBar.getChildCount() < 2) {
					inputBar.recycle();
					break;
				}

				AccessibilityNodeInfo inputNode = inputBar.getChild(0);
				AccessibilityNodeInfo sendNode = inputBar.getChild(1);

				setUnpackPending(true);
				CavanAccessibility.setNodeText(mService, inputNode, message);
				parent.performAction(AccessibilityNodeInfo.ACTION_CLICK);
				sendNode.performAction(AccessibilityNodeInfo.ACTION_CLICK);

				sendNode.recycle();
				inputNode.recycle();
				inputBar.recycle();
				parent.recycle();
				return true;
			}

			parent.recycle();
		}

		return false;
	}

	public String findPacket(String title) {
		for (String packet : mPackets) {
			if (packet.contains(title) || title.contains(packet)) {
				return packet;
			}
		}

		return null;
	}

	private long doFindAndUnpack(AccessibilityNodeInfo root) {
		AccessibilityNodeInfo backNode = CavanAccessibility.findNodeByViewId(root, "com.tencent.mobileqq:id/ivTitleBtnLeft");
		if (backNode != null) {
			String title = CavanAccessibility.getNodeTextByViewId(root, "com.tencent.mobileqq:id/title");
			CavanAndroid.dLog("title = " + title);

			if (title != null) {
				String packet = findPacket(title);
				CavanAndroid.dLog("packet = " + packet);

				if (packet != null) {
					if (doAutoUnpack(root)) {
						backNode.recycle();
						return POLL_DELAY;
					}

					CavanAndroid.dLog("complete: " + packet);
					mPackets.remove(packet);
					resetRetryCount();
				}
			}

			backNode.performAction(AccessibilityNodeInfo.ACTION_CLICK);
			backNode.recycle();
		} else {
			AccessibilityNodeInfo listNode = CavanAccessibility.findNodeByViewId(root, "com.tencent.mobileqq:id/recent_chat_list");
			if (listNode != null) {
				for (int i = 0; i < 10 && listNode.performAction(AccessibilityNodeInfo.ACTION_SCROLL_BACKWARD); i++) {
					CavanAndroid.dLog("ACTION_SCROLL_BACKWARD" + i);

					try {
						Thread.sleep(50);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}

				if (++mChildIndex < listNode.getChildCount()) {
					CavanAndroid.dLog("mChildIndex = " + mChildIndex);
					CavanAccessibility.performChildActionClick(listNode, mChildIndex);
				} else {
					CavanAndroid.dLog("mRetryCount = " + mRetryCount);

					if (++mRetryCount > MAX_RETRY_COUNT) {
						CavanAndroid.dLog("not found: " + mPackets);
						mPackets.clear();
					}

					mChildIndex = 0;
				}

				listNode.recycle();
			} else {
				performGlobalAction(AccessibilityService.GLOBAL_ACTION_BACK);
				return POLL_DELAY_BACK;
			}
		}

		return POLL_DELAY;
	}

	private long doFindAndUnpack() {
		CavanAndroid.dLog("mUnpackPending = " + mUnpackPending);

		if (mUnpackPending) {
			return POLL_DELAY;
		}

		AccessibilityNodeInfo root = getRootInActiveWindow();
		if (root == null) {
			return POLL_DELAY;
		}

		CharSequence pkgName = root.getPackageName();
		CavanAndroid.dLog("package = " + pkgName);

		if (CavanPackageName.QQ.equals(pkgName) || CavanPackageName.QWALLET.equals(pkgName)) {
			if ("com.tencent.mobileqq.activity.SplashActivity".equals(mClassName)) {
				if (mPackets.isEmpty()) {
					if (!mService.startNextPendingActivity()) {
						mService.startIdleActivity();
					}
				} else {
					return doFindAndUnpack(root);
				}
			} else {
				performGlobalAction(AccessibilityService.GLOBAL_ACTION_BACK);
				return POLL_DELAY_BACK;
			}
		} else {
			return 0;
		}

		return POLL_DELAY;
	}

	@Override
	public String getPackageName() {
		return CavanPackageName.QQ;
	}

	@Override
	public int getRedPacketCount() {
		return mPackets.size();
	}

	@Override
	public void handleMessage(Message msg) {
		switch (msg.what) {
		case MSG_ADD_PACKET:
			mPackets.add((String) msg.obj);
			resetRetryCount();
			startAutoUnpack();
			break;

		case MSG_UNPACK_TIMEOUT:
			if (mUnpackPending) {
				mUnpackPending = false;
				startAutoUnpack();
			}
			break;
		}
	}

	@Override
	public void onWindowStateChanged(AccessibilityEvent event) {
		switch (mClassName) {
		case "com.tencent.mobileqq.activity.SplashActivity":
			if (getRedPacketCount() > 0) {
				startAutoUnpack();
			}
			break;

		case "cooperation.qwallet.plugin.QWalletPluginProxyActivity":
			if (getRedPacketCount() > 0) {
				setUnpackPending(false);
			}
			break;

		case "com.tencent.mobileqq.activity.aio.photo.AIOGalleryActivity":
			mService.setAutoOpenAppEnable(false);
			break;
		}
	}

	@Override
	public void onWindowContentChanged(AccessibilityEvent event) {
		AccessibilityNodeInfo source = event.getSource();
		if (source != null && isMessageBoxNode(source)) {
			CharSequence sequence = source.getText();
			if (sequence == null) {
				return;
			}

			String text = sequence.toString();
			if (text.equals(mMessageBoxText)) {
				return;
			}

			mMessageBoxText = text;

			if (text.contains("[QQ红包]")) {
				source.performAction(AccessibilityNodeInfo.ACTION_CLICK);
			}

			if (!CavanAndroid.inKeyguardRestrictedInputMode(mService)) {
				Intent intent = new Intent(MainActivity.ACTION_CONTENT_RECEIVED);
				intent.putExtra("package", source.getPackageName());
				intent.putExtra("desc", "QQ消息盒子");
				intent.putExtra("content", text);
				intent.putExtra("hasPrefix", true);
				mService.sendBroadcast(intent);
			}
		}
	}

	@Override
	public void onViewClicked(AccessibilityEvent event) {
		AccessibilityNodeInfo source = event.getSource();
		if (source == null) {
			return;
		}

		if (CavanAccessibility.isTextView(source)) {
			AccessibilityNodeInfo parent = source.getParent();
			if (parent == null) {
				return;
			}

			String id = parent.getViewIdResourceName();
			if (id == null) {
				return;
			}

			if (id.equals("com.tencent.mobileqq:id/chat_item_content_layout") ||
					id.equals("com.tencent.mobileqq:id/name")) {
				postMessageNode(source);
			}
		}
	}
}
