package com.cavan.cavanmain;

import java.util.LinkedList;
import java.util.Queue;

import android.accessibilityservice.AccessibilityService;
import android.content.Intent;
import android.graphics.Rect;
import android.os.Build;
import android.os.Message;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;

public class CavanAccessibilityQQ extends CavanAccessibilityBase {

	private static final int POLL_DELAY = 500;
	private static final int POLL_DELAY_BACK = 2000;
	private static final int MSG_ADD_PACKET = 1;

	private String mMessageBoxText;

	private Queue<String> mQueue = new LinkedList<String>();
	private Runnable mRunnablePoll = new Runnable() {

		@Override
		public void run() {
			removeCallbacks(this);

			AccessibilityNodeInfo root = getRootInActiveWindow();
			if (root != null) {
				CharSequence pkgName = root.getPackageName();
				CavanAndroid.dLog("package = " + pkgName);

				if (CavanPackageName.QQ.equals(pkgName) || CavanPackageName.QWALLET.equals(pkgName)) {
					if ("com.tencent.mobileqq.activity.SplashActivity".equals(mClassName)) {
						String name = mQueue.peek();
						CavanAndroid.dLog("queue: name = " + name);

						if (name == null) {
							if (!mService.startNextPendingActivity()) {
								mService.startIdleActivity();
							}
						} else {
							doAutoUnpack(root, name);
						}
					} else {
						performGlobalActionBack();
					}
				} else {
					return;
				}
			}

			startAutoUnpack();
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

		if (CavanAccessibilityService.isTextView(node)) {
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

	private void startAutoUnpack() {
		startAutoUnpack(POLL_DELAY);
	}

	private void performGlobalActionBack() {
		CavanAndroid.dLog("performGlobalActionBack");

		removeCallbacks(mRunnablePoll);
		postDelayed(mRunnablePoll, POLL_DELAY_BACK);
		performGlobalAction(AccessibilityService.GLOBAL_ACTION_BACK);
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
		for (AccessibilityNodeInfo node : root.findAccessibilityNodeInfosByText("QQ红包")) {
			AccessibilityNodeInfo parent = node.getParent();
			if (parent == null || parent.getChildCount() < 3) {
				continue;
			}

			CharSequence sequence = parent.getChild(2).getText();
			if (sequence == null || "QQ红包".equals(sequence.toString()) == false) {
				continue;
			}

			sequence = parent.getChild(0).getText();
			if (sequence == null) {
				continue;
			}

			String message = sequence.toString();
			if (message.contains("测") || message.contains("挂")) {
				continue;
			}

			CavanAndroid.dLog("message = " + message);

			sequence = parent.getChild(1).getText();
			if (sequence == null) {
				continue;
			}

			switch (sequence.toString()) {
			case "点击拆开":
				parent.performAction(AccessibilityNodeInfo.ACTION_CLICK);
				return true;

			case "口令红包":
				AccessibilityNodeInfo inputBar = CavanAccessibilityService.findAccessibilityNodeInfoByViewId(root, "com.tencent.mobileqq:id/inputBar");
				if (inputBar == null || inputBar.getChildCount() < 2) {
					break;
				}

				AccessibilityNodeInfo inputNode = inputBar.getChild(0);
				AccessibilityNodeInfo sendNode = inputBar.getChild(1);

				mService.setAccessibilityNodeText(inputNode, message);
				parent.performAction(AccessibilityNodeInfo.ACTION_CLICK);
				sendNode.performAction(AccessibilityNodeInfo.ACTION_CLICK);
				return true;
			}
		}

		return false;
	}

	private AccessibilityNodeInfo findUnreadNode(AccessibilityNodeInfo listNode) {
		for (AccessibilityNodeInfo node : listNode.findAccessibilityNodeInfosByViewId("com.tencent.mobileqq:id/relativeItem")) {
			AccessibilityNodeInfo unreadNode = CavanAccessibilityService.findAccessibilityNodeInfoByViewId(node, "com.tencent.mobileqq:id/unreadmsg");
			if (unreadNode == null) {
				continue;
			}

			try {
				if (Integer.parseInt(unreadNode.getText().toString()) > 0) {
					return node;
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		}

		return null;
	}

	private void doAutoUnpack(AccessibilityNodeInfo root, String name) {
		AccessibilityNodeInfo backNode = CavanAccessibilityService.findAccessibilityNodeInfoByViewId(root, "com.tencent.mobileqq:id/ivTitleBtnLeft");
		if (backNode != null) {
			AccessibilityNodeInfo titleNode = CavanAccessibilityService.findAccessibilityNodeInfoByViewId(root, "com.tencent.mobileqq:id/title");
			if (titleNode != null) {
				CharSequence sequence = titleNode.getText();
				if (sequence != null) {
					String title = sequence.toString();
					CavanAndroid.dLog("title = " + title);

					if (CavanChatNode.isMatch(title, name)) {
						if (doAutoUnpack(root)) {
							return;
						}

						CavanAndroid.dLog("complete: " + name);
						mQueue.remove();
					}
				}
			}

			backNode.performAction(AccessibilityNodeInfo.ACTION_CLICK);
		} else {
			AccessibilityNodeInfo listNode = CavanAccessibilityService.findAccessibilityNodeInfoByViewId(root, "com.tencent.mobileqq:id/recent_chat_list");
			if (listNode != null) {
				AccessibilityNodeInfo node = findUnreadNode(listNode);

				if (node != null) {
					AccessibilityNodeInfo parent = node.getParent();
					if (parent != null) {
						parent.performAction(AccessibilityNodeInfo.ACTION_CLICK);
					}
				} else {
					CavanAndroid.dLog("not found: " + name);
					mQueue.remove();
				}
			} else {
				performGlobalActionBack();
			}
		}
	}

	@Override
	public String getPackageName() {
		return CavanPackageName.QQ;
	}

	@Override
	public int getRedPacketCount() {
		return mQueue.size();
	}

	@Override
	public void handleMessage(Message msg) {
		switch (msg.what) {
		case MSG_ADD_PACKET:
			mQueue.add((String) msg.obj);
			startAutoUnpack();
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
				performGlobalActionBack();
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

			if (text.contains("[QQ红包]") && getRedPacketCount() == 0) {
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

		if (CavanAccessibilityService.isTextView(source)) {
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
