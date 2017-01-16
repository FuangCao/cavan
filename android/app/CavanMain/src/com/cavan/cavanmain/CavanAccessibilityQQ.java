package com.cavan.cavanmain;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

import android.content.Intent;
import android.graphics.Rect;
import android.os.Build;
import android.os.Message;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanAccessibility;
import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;
import com.cavan.android.DelayedRunnable;
import com.cavan.java.CavanJava;

public class CavanAccessibilityQQ extends CavanAccessibilityBase {

	private static final String RED_PACKET_NAME = "QQ红包";

	private static final int POLL_DELAY = 200;
	private static final int POLL_DELAY_JUMP = 500;
	private static final int POLL_DELAY_UNPACK = 500;
	private static final int SCROLL_DELAY = 200;
	private static final int MAX_SCROLL_COUNT = 2;
	private static final int MAX_RETRY_COUNT = 3;

	private int mChatIndex;
	private int mRetryCount;
	private boolean mUnpackPending;
	private String mMessageBoxText;

	private List<Integer> mMesssages = new ArrayList<Integer>();
	private LinkedList<String> mPackets = new LinkedList<String>();
	private Runnable mRunnablePoll = new Runnable() {

		@Override
		public void run() {
			removeCallbacks(this);

			int count = mService.getMessageCount();
			if (count <= 0) {
				CavanAndroid.dLog("getMessageCount = " + count);
				mMesssages.clear();
			} else if (isLocked()) {
				CavanAndroid.dLog("isLocked");
			} else {
				long delay = doFindAndUnpack();
				if (delay > 0) {
					postDelayed(this, delay);
				}
			}
		}
	};

	private DelayedRunnable mRunnableClick = new DelayedRunnable(this) {

		@Override
		protected void onRunableFire() {
			mHandler.post(mRunnablePoll);
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

	private AccessibilityNodeInfo getRedPacketBackNode(AccessibilityNodeInfo root) {
		AccessibilityNodeInfo backNode = CavanAccessibility.findNodeByViewId(root, "com.tencent.mobileqq:id/ivTitleBtnLeft");
		if (backNode != null) {
			return backNode;
		}

		for (AccessibilityNodeInfo node : CavanAccessibility.findNodesByClassName(root, CavanAccessibility.CLASS_IMAGE_BUTTON)) {
			if (backNode == null && "关闭".equals(node.getContentDescription())) {
				backNode = node;
			} else {
				node.recycle();
			}
		}

		return backNode;
	}

	private void setUnpackPending(AccessibilityNodeInfo root, boolean pending) {
		CavanAndroid.dLog("setUnpackBusy: " + mUnpackPending + " -> " + pending);

		if (pending) {
			mUnpackPending = true;
			setLockEnable(POLL_DELAY_UNPACK, false);
		} else if (mUnpackPending) {
			AccessibilityNodeInfo backNode = getRedPacketBackNode(root);
			if (backNode != null) {
				mUnpackPending = false;
				CavanAccessibility.performClick(backNode);
			}

			setLockEnable(POLL_DELAY, false);
		}
	}

	public void addRedPacket(String name) {
		int delay = MainActivity.getAutoUnpackQQ(mService);
		if (delay < 0) {
			return;
		}

		if (mPackets.contains(name)) {
			return;
		}

		mPackets.add(name);
		mChatIndex = 0;
		mRetryCount = 0;
		setLockEnable(POLL_DELAY, false);
	}

	private boolean doUnpackGeneral(AccessibilityNodeInfo root, AccessibilityNodeInfo node) {
		if (node.getChildCount() < 2) {
			return false;
		}

		String message = CavanAccessibility.getChildText(node, 0);
		CavanAndroid.dLog("message = " + message);

		if (message == null || message.contains("测") || message.contains("挂")) {
			return false;
		}

		String state = CavanAccessibility.getChildText(node, 1);
		CavanAndroid.dLog("state = " + state);

		if (state == null) {
			return false;
		}

		switch (state) {
		case "点击拆开":
		case "QQ红包个性版":
			setUnpackPending(root, true);
			CavanAccessibility.performClick(node);
			return true;

		case "口令红包":
			if (!CavanAccessibility.performClick(node)) {
				break;
			}

			AccessibilityNodeInfo inputBar = CavanAccessibility.findNodeByViewId(root, "com.tencent.mobileqq:id/inputBar");
			if (inputBar == null || inputBar.getChildCount() < 2) {
				inputBar.recycle();
				break;
			}

			if (!CavanAccessibility.setChildText(mService, inputBar, 0, message)) {
				inputBar.recycle();
				break;
			}

			setUnpackPending(root, true);
			CavanAccessibility.performChildClick(inputBar, 1);
			inputBar.recycle();
			return true;
		}

		return false;
	}

	private boolean doAutoUnpack(AccessibilityNodeInfo root, AccessibilityNodeInfo listNode) {
		List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByText(RED_PACKET_NAME);
		if (nodes == null) {
			return false;
		}

		int index;
		boolean success = false;

		for (index = nodes.size() - 1; index >= 0; index--) {
			AccessibilityNodeInfo node = nodes.get(index);
			int hash = node.hashCode();

			if (!mMesssages.contains(hash)) {
				String type = CavanAccessibility.getNodeText(node);
				CavanAndroid.dLog("type = " + type);

				if (type.startsWith(RED_PACKET_NAME)) {
					AccessibilityNodeInfo parent = node.getParent();
					if (parent != null) {
						success = doUnpackGeneral(root, parent);
						parent.recycle();
						if (success) {
							mMesssages.add(hash);
							break;
						}
					}
				}
			}

			node.recycle();
		}

		while (index >= 0) {
			nodes.get(index--).recycle();
		}

		return success;
	}

	private boolean doAutoUnpack(AccessibilityNodeInfo root) {
		AccessibilityNodeInfo listNode = CavanAccessibility.findNodeByViewId(root, "com.tencent.mobileqq:id/listView1");
		if (listNode == null) {
			listNode = CavanAccessibility.findChildByClassName(root, CavanAccessibility.CLASS_ABS_LIST_VIEW);
			if (listNode == null) {
				return false;
			}
		}

		if (mMesssages.isEmpty() && CavanAccessibility.performScrollDown(listNode, 100)) {
			CavanJava.msleep(SCROLL_DELAY);
		}

		for (int i = 0; i < MAX_SCROLL_COUNT; i++) {
			if (doAutoUnpack(root, listNode)) {
				listNode.recycle();
				return true;
			}

			if (CavanAccessibility.performScrollUp(listNode)) {
				CavanAndroid.dLog("performScrollUp" + i);
				CavanJava.msleep(SCROLL_DELAY);
			} else {
				break;
			}
		}

		for (int i = 0; i < 10 && CavanAccessibility.performScrollDown(listNode); i++) {
			CavanAndroid.dLog("performScrollDown" + i);

			CavanJava.msleep(SCROLL_DELAY);

			if (doAutoUnpack(root, listNode)) {
				listNode.recycle();
				return true;
			}
		}

		listNode.recycle();

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
			mRunnableClick.cancel();

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

					while (mPackets.remove(packet)) {
						CavanAndroid.dLog("complete: " + packet);
					}

					mMesssages.clear();
					mRetryCount = 0;
				}
			}

			CavanAccessibility.performClickAndRecycle(backNode);
			return POLL_DELAY;
		} else {
			if (mRunnableClick.isEnabled()) {
				CavanAndroid.dLog("mRunnableClick.isEnabled()");
				return POLL_DELAY;
			}

			AccessibilityNodeInfo listNode = CavanAccessibility.findNodeByViewId(root, "com.tencent.mobileqq:id/recent_chat_list");
			if (listNode == null) {
				performGlobalBackDelayed(POLL_DELAY_JUMP);
				return POLL_DELAY;
			}

			mMesssages.clear();
			cancelGlobalBack();

			Rect bounds = CavanAccessibility.getBoundsInScreen(listNode);
			if (bounds.left > 0) {
				listNode.recycle();
				performGlobalBack(POLL_DELAY);
				return 0;
			}

			CavanAccessibility.performScrollUp(listNode, 10);
			List<AccessibilityNodeInfo> chats = null;

			while (true) {
				if (chats == null || chats.isEmpty()) {
					chats = listNode.findAccessibilityNodeInfosByViewId("com.tencent.mobileqq:id/relativeItem");
				}

				if (chats != null && mChatIndex < chats.size()) {
					CavanAndroid.dLog("mChatIndex = " + mChatIndex);

					AccessibilityNodeInfo parent = chats.get(mChatIndex++).getParent();
					if (parent != null) {
						if (parent.hashCode() != listNode.hashCode()) {
							if (mRetryCount == 0 || CavanAccessibility.getNodeCountByViewId(parent, "com.tencent.mobileqq:id/unreadmsg") > 0) {
								mRunnableClick.post(POLL_DELAY_JUMP, false);
								CavanAccessibility.performClickAndRecycle(parent);
								break;
							}
						}

						parent.recycle();
					}
				} else {
					CavanAndroid.dLog("mRetryCount = " + mRetryCount);

					mChatIndex = 0;

					if (++mRetryCount > MAX_RETRY_COUNT) {
						CavanAndroid.dLog("not found: " + mPackets);
						mPackets.clear();
						mRetryCount = 0;
						break;
					}
				}
			}

			if (chats != null) {
				CavanAccessibility.recycleNodes(chats);
			}

			listNode.recycle();

			return POLL_DELAY;
		}
	}

	private long doFindAndUnpack() {
		AccessibilityNodeInfo root = getRootInActiveWindow();
		if (root == null) {
			return POLL_DELAY;
		}

		CharSequence pkgName = root.getPackageName();
		CavanAndroid.dLog("package = " + pkgName);

		if (CavanPackageName.QQ.equals(pkgName) || CavanPackageName.QWALLET.equals(pkgName)) {
			CavanAndroid.dLog("mClassName = " + mClassName);

			switch (mClassName) {
			case "com.tencent.mobileqq.activity.SplashActivity":
				if (mPackets.isEmpty()) {
					if (!mService.startNextPendingActivity()) {
						mService.startIdleActivity();
					}
				} else {
					return doFindAndUnpack(root);
				}
				break;

			case "cooperation.qwallet.plugin.QWalletPluginProxyActivity":
				setUnpackPending(root, false);
				break;

			default:
				performGlobalBack(POLL_DELAY_JUMP);
				return 0;
			}
		} else {
			return 0;
		}

		return POLL_DELAY;
	}

	@Override
	protected void onLockStateChanged(boolean locked) {
		CavanAndroid.dLog("onLockStateChanged: locked = " + locked);

		if (locked) {
			removeCallbacks(mRunnablePoll);
		} else {
			post(mRunnablePoll);
		}
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
	public void clearRedPackets() {
		mMesssages.clear();
	}

	@Override
	public void handleMessage(Message msg) {
		CavanAccessibility.dumpNode(getRootInActiveWindow());
	}

	@Override
	public void onWindowStateChanged(AccessibilityEvent event) {
		switch (mClassName) {
		case "com.tencent.mobileqq.activity.SplashActivity":
			if (getRedPacketCount() > 0) {
				setLockEnable(POLL_DELAY, false);
			}
			break;

		case "cooperation.qwallet.plugin.QWalletPluginProxyActivity":
			if (getRedPacketCount() > 0) {
				setLockEnable(POLL_DELAY, true);
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
