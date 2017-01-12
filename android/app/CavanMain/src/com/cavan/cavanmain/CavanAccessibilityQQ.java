package com.cavan.cavanmain;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

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

	private static final int POLL_DELAY_FAST = 200;
	private static final int POLL_DELAY_SLOW = 500;
	private static final int POLL_DELAY_BACK = 2000;
	private static final int POLL_DELAY_UNPACK = 1000;

	private static final int MAX_RETRY_COUNT = 3;

	private static final int MSG_ADD_PACKET = 1;
	private static final int MSG_UNPACK_TIMEOUT = 2;

	private int mChatIndex;
	private int mRetryCount;
	private boolean mUnpackBusy;
	private String mMessageBoxText;

	private List<Integer> mMesssages = new ArrayList<Integer>();
	private LinkedList<String> mPackets = new LinkedList<String>();
	private Runnable mRunnablePoll = new Runnable() {

		@Override
		public void run() {
			removeCallbacks(this);

			CavanAndroid.dLog("mUnpackBusy = " + mUnpackBusy);

			if (mUnpackBusy) {
				return;
			}

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

	private void startAutoUnpack(long delay) {
		postDelayed(mRunnablePoll, delay);
	}

	private void stopAutoUnpack() {
		removeCallbacks(mRunnablePoll);
	}

	private void startAutoUnpack() {
		startAutoUnpack(POLL_DELAY_SLOW);
	}

	private void setUnpackBusy(boolean busy) {
		removeMessages(MSG_UNPACK_TIMEOUT);
		mUnpackBusy = busy;

		if (busy) {
			stopAutoUnpack();
			sendEmptyMessageDelayed(MSG_UNPACK_TIMEOUT, POLL_DELAY_UNPACK);
		} else {
			startAutoUnpack();
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
			setUnpackBusy(true);
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

			setUnpackBusy(true);
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
				mMesssages.add(hash);

				String type = CavanAccessibility.getNodeText(node);
				CavanAndroid.dLog("type = " + type);

				if (type.startsWith(RED_PACKET_NAME)) {
					AccessibilityNodeInfo parent = node.getParent();
					if (parent != null) {
						success = doUnpackGeneral(root, parent);
						parent.recycle();
						if (success) {
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
			listNode = CavanAccessibility.findChildByClassName(root, CavanAccessibility.CLASS_ABSLISTVIEW);
			if (listNode == null) {
				return false;
			}
		}

		CavanAccessibility.performScrollDown(listNode, 10);

		for (int i = 0; i < 3; i++) {
			if (doAutoUnpack(root, listNode)) {
				listNode.recycle();
				return true;
			}

			if (CavanAccessibility.performScrollUp(listNode, 1)) {
				try {
					Thread.sleep(100);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			} else {
				break;
			}
		}

		CavanAccessibility.performScrollDown(listNode, 10);

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

	private List<AccessibilityNodeInfo> getRecentChats(AccessibilityNodeInfo root) {
		AccessibilityNodeInfo listNode = CavanAccessibility.findNodeByViewId(root, "com.tencent.mobileqq:id/recent_chat_list");
		if (listNode == null) {
			return null;
		}

		CavanAccessibility.performScrollUp(listNode, 10);

		List<AccessibilityNodeInfo> chats = listNode.findAccessibilityNodeInfosByViewId("com.tencent.mobileqq:id/relativeItem");
		listNode.recycle();
		return chats;
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
						return POLL_DELAY_SLOW;
					}

					CavanAndroid.dLog("complete: " + packet);
					mPackets.remove(packet);
					mRetryCount = 0;
				}
			}

			CavanAccessibility.performClickAndRecycle(backNode);
			return POLL_DELAY_FAST;
		} else {
			List<AccessibilityNodeInfo> chats = getRecentChats(root);
			if (chats == null || chats.isEmpty()) {
				performGlobalAction(AccessibilityService.GLOBAL_ACTION_BACK);
				return POLL_DELAY_BACK;
			}

			CavanAndroid.dLog("chats = " + chats.size());

			while (true) {
				if (mChatIndex < chats.size()) {
					CavanAndroid.dLog("mChatIndex = " + mChatIndex);

					AccessibilityNodeInfo parent = chats.get(mChatIndex++).getParent();
					if (mRetryCount == 0 || CavanAccessibility.getNodeCountByViewId(parent, "com.tencent.mobileqq:id/unreadmsg") > 0) {
						mMesssages.clear();
						CavanAccessibility.performClickAndRecycle(parent);
						break;
					}

					parent.recycle();
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

			CavanAccessibility.recycleNodes(chats);

			return POLL_DELAY_SLOW;
		}
	}

	private long doFindAndUnpack() {
		AccessibilityNodeInfo root = getRootInActiveWindow();
		if (root == null) {
			return POLL_DELAY_SLOW;
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

		return POLL_DELAY_SLOW;
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
			mChatIndex = 0;
			mRetryCount = 0;
			startAutoUnpack();
			break;

		case MSG_UNPACK_TIMEOUT:
			if (mUnpackBusy) {
				mUnpackBusy = false;
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
				setUnpackBusy(false);
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
