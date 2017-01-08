package com.cavan.cavanmain;

import java.util.LinkedList;
import java.util.List;
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

	private static final int POLL_DELAY = 1000;
	private static final int MSG_ADD_PACKET = 1;

	private String mMessageBoxText;
	private AccessibilityNodeInfo mCurrNode;
	private CavanChatMap mChatMap = new CavanChatMap();
	private Queue<String> mQueue = new LinkedList<String>();
	private Runnable mRunnablePoll = new Runnable() {

		@Override
		public void run() {
			removeCallbacks(this);

			AccessibilityNodeInfo root = getRootInActiveWindow();
			if (root == null || CavanPackageName.QQ.equals(root.getPackageName()) == false) {
				return;
			}

			if ("com.tencent.mobileqq.activity.SplashActivity".equals(mClassName)) {
				String name = mQueue.peek();
				if (name == null) {
					performGlobalAction(AccessibilityService.GLOBAL_ACTION_HOME);
				} else {
					doAutoUnpack(root, name);
				}
			} else {
				performGlobalAction(AccessibilityService.GLOBAL_ACTION_BACK);
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

	private void startAutoUnpack() {
		postDelayed(mRunnablePoll, POLL_DELAY);
	}

	public void addRedPacket(String name) {
		int delay = MainActivity.getAutoUnpackQQ(mService);
		if (delay < 0) {
			return;
		}

		Message message = obtainMessage(MSG_ADD_PACKET, name);
		sendMessageDelayed(message, delay * 1000);
	}

	private int doAutoUnpack(AccessibilityNodeInfo root) {
		int count = 0;

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
				count++;
				break;

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
				count++;
				break;
			}
		}

		return count;
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

					if (mCurrNode != null) {
						mChatMap.put(mCurrNode, title);
						mCurrNode = null;
					}

					if (CavanChatNode.isMatch(title, name) && doAutoUnpack(root) == 0) {
						mQueue.remove();
						return;
					}
				}
			}

			backNode.performAction(AccessibilityNodeInfo.ACTION_CLICK);
		} else {
			AccessibilityNodeInfo listNode = CavanAccessibilityService.findAccessibilityNodeInfoByViewId(root, "com.tencent.mobileqq:id/recent_chat_list");
			if (listNode != null) {
				boolean not_found = true;

				for (AccessibilityNodeInfo node : listNode.findAccessibilityNodeInfosByViewId("com.tencent.mobileqq:id/relativeItem")) {
					AccessibilityNodeInfo parent = node.getParent();
					if (parent == null) {
						continue;
					}

					CavanChatNode chat = mChatMap.get(parent);
					if (chat == null || chat.isMatch(name)) {
						not_found = false;
						mCurrNode = parent;
						parent.performAction(AccessibilityNodeInfo.ACTION_CLICK);
						break;
					}
				}

				if (not_found) {
					CavanAndroid.dLog("not found: " + name);
					mQueue.remove();
				}
			} else {
				if (mCurrNode != null) {
					mChatMap.put(mCurrNode, "Invalid");
					mCurrNode = null;
				}

				performGlobalAction(AccessibilityService.GLOBAL_ACTION_BACK);
			}
		}
	}

	@Override
	public String getPackageName() {
		return CavanPackageName.QQ;
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
		mCurrNode = null;

		switch (mClassName) {
		case "com.tencent.mobileqq.activity.SplashActivity":
			if (mQueue.size() > 0) {
				startAutoUnpack();
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
