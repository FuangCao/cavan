package com.cavan.cavanmain;

import java.util.ArrayList;
import java.util.List;

import android.graphics.Rect;
import android.os.Build;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanAccessibilityHelper;
import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;
import com.cavan.android.DelayedRunnable;
import com.cavan.java.CavanJava;

public class CavanAccessibilityQQ extends CavanAccessibilityBase<String> {

	private static final String RED_PACKET_NAME = "QQ红包";

	private static final int POLL_DELAY = 200;
	private static final int POLL_DELAY_JUMP = 1000;
	private static final int POLL_DELAY_UNPACK = 2000;
	private static final int SCROLL_DELAY = 200;
	private static final int MAX_SCROLL_COUNT = 2;
	private static final int MAX_RETRY_COUNT = 3;

	private static CavanAccessibilityQQ sInstance;

	public static CavanAccessibilityQQ getInstance() {
		return sInstance;
	}

	private int mChatIndex;
	private int mRetryCount;
	private int mUnpackTimes;
	private boolean mUnpackPending;
	private String mMessageBoxText;
	private List<Integer> mFinishNodes = new ArrayList<Integer>();

	private DelayedRunnable mRunnableClick = new DelayedRunnable(this) {

		@Override
		protected void onRunableFire() {
			setLockEnable(POLL_DELAY, true);
		}
	};

	public CavanAccessibilityQQ(CavanAccessibilityService service) {
		super(service);
		sInstance = this;
	}

	public boolean isMessageBoxNode(AccessibilityNodeInfo node) {
		if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
			String id = node.getViewIdResourceName();
			return "com.tencent.mobileqq:id/msgbox".equals(id);
		}

		if (node.isMultiLine()) {
			return false;
		}

		if (CavanAccessibilityHelper.isTextView(node)) {
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
		AccessibilityNodeInfo backNode = CavanAccessibilityHelper.findNodeByViewId(root, "com.tencent.mobileqq:id/ivTitleBtnLeft");
		if (backNode != null) {
			return backNode;
		}

		for (AccessibilityNodeInfo node : CavanAccessibilityHelper.findNodesByClassName(root, CavanAccessibilityHelper.CLASS_IMAGE_BUTTON)) {
			if (backNode == null && "关闭".equals(node.getContentDescription())) {
				backNode = node;
			} else {
				node.recycle();
			}
		}

		return backNode;
	}

	private void setUnpackPending(AccessibilityNodeInfo root, boolean pending) {
		CavanAndroid.dLog("setUnpackPending: " + mUnpackPending + " -> " + pending);
		CavanAndroid.dLog("mUnpackTimes = " + mUnpackTimes);

		if (pending) {
			mUnpackTimes = 0;
			mUnpackPending = true;
			setLockEnable(POLL_DELAY_UNPACK, false);
		} else if (mUnpackPending) {
			AccessibilityNodeInfo backNode = getRedPacketBackNode(root);
			if (backNode != null) {
				mUnpackTimes = 0;
				mUnpackPending = false;
				CavanAccessibilityHelper.performClick(backNode);
			}

			setLockEnable(POLL_DELAY, false);
		} else if (mUnpackTimes > 10) {
			mUnpackTimes = 0;
			performGlobalBack(POLL_DELAY);
		} else {
			mUnpackTimes++;
		}
	}

	private boolean doUnpackGeneral(AccessibilityNodeInfo root, AccessibilityNodeInfo node) {
		if (node.getChildCount() < 2) {
			return false;
		}

		String message = CavanAccessibilityHelper.getChildText(node, 0);
		CavanAndroid.dLog("message = " + message);

		if (message == null || message.contains("测") || message.contains("挂")) {
			return false;
		}

		String state = CavanAccessibilityHelper.getChildText(node, 1);
		CavanAndroid.dLog("state = " + state);

		if (state == null) {
			return false;
		}

		switch (state) {
		case "点击拆开":
		case "QQ红包个性版":
			setUnpackPending(root, true);
			CavanAccessibilityHelper.performClick(node);
			return true;

		case "口令红包":
			if (!CavanAccessibilityHelper.performClick(node)) {
				break;
			}

			AccessibilityNodeInfo inputBar = CavanAccessibilityHelper.findNodeByViewId(root, "com.tencent.mobileqq:id/inputBar");
			if (inputBar == null || inputBar.getChildCount() < 2) {
				inputBar.recycle();
				break;
			}

			if (CavanAccessibilityHelper.setChildText(mService, inputBar, 0, message) == null) {
				inputBar.recycle();
				break;
			}

			setUnpackPending(root, true);
			CavanAccessibilityHelper.performChildClick(inputBar, 1);
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

			if (mFinishNodes.contains(hash)) {
				continue;
			}

			String type = CavanAccessibilityHelper.getNodeText(node);
			CavanAndroid.dLog("type = " + type);

			if (type.startsWith(RED_PACKET_NAME)) {
				AccessibilityNodeInfo parent = node.getParent();
				if (parent != null) {
					success = doUnpackGeneral(root, parent);
					parent.recycle();
					if (success) {
						mFinishNodes.add(hash);
						break;
					}
				}
			}
		}

		CavanAccessibilityHelper.recycleNodes(nodes);

		return success;
	}

	private boolean doAutoUnpack(AccessibilityNodeInfo root) {
		AccessibilityNodeInfo listNode = CavanAccessibilityHelper.findNodeByViewId(root, "com.tencent.mobileqq:id/listView1");
		if (listNode == null) {
			listNode = CavanAccessibilityHelper.findChildByClassName(root, CavanAccessibilityHelper.CLASS_ABS_LIST_VIEW);
			if (listNode == null) {
				return false;
			}
		}

		if (mFinishNodes.isEmpty() && CavanAccessibilityHelper.performScrollDown(listNode, 100)) {
			CavanJava.msleep(SCROLL_DELAY);
		}

		for (int i = 0; i < MAX_SCROLL_COUNT; i++) {
			if (doAutoUnpack(root, listNode)) {
				listNode.recycle();
				return true;
			}

			if (CavanAccessibilityHelper.performScrollUp(listNode)) {
				CavanAndroid.dLog("performScrollUp" + i);
				CavanJava.msleep(SCROLL_DELAY);
			} else {
				break;
			}
		}

		for (int i = 0; i < 10 && CavanAccessibilityHelper.performScrollDown(listNode); i++) {
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

	private AccessibilityNodeInfo findBackNode(AccessibilityNodeInfo root) {
		AccessibilityNodeInfo node = CavanAccessibilityHelper.findNodeByViewId(root, "com.tencent.mobileqq:id/ivTitleBtnLeft");
		if (node != null) {
			return node;
		}

		AccessibilityNodeInfo parent = CavanAccessibilityHelper.findNodeByViewId(root, "com.tencent.mobileqq:id/rlCommenTitle");
		if (parent == null) {
			return null;
		}

		if (parent.getChildCount() > 0) {
			node = parent.getChild(0);
		}

		parent.recycle();

		return node;
	}

	private long doFindAndUnpack(AccessibilityNodeInfo root) {
		AccessibilityNodeInfo backNode = findBackNode(root);
		if (backNode != null) {
			mRunnableClick.cancel();

			String title = CavanAccessibilityHelper.getNodeTextByViewId(root, "com.tencent.mobileqq:id/title");
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

					mFinishNodes.clear();
					mRetryCount = 0;
				}
			}

			CavanAccessibilityHelper.performClickAndRecycle(backNode);

			return POLL_DELAY;
		} else {
			if (mRunnableClick.isEnabled()) {
				CavanAndroid.dLog("mRunnableClick.isEnabled()");
				return POLL_DELAY;
			}

			AccessibilityNodeInfo listNode = CavanAccessibilityHelper.findNodeByViewId(root, "com.tencent.mobileqq:id/recent_chat_list");
			if (listNode == null) {
				performGlobalBackDelayed(POLL_DELAY_JUMP);
				return POLL_DELAY;
			}

			mFinishNodes.clear();
			cancelGlobalBack();

			Rect bounds = CavanAccessibilityHelper.getBoundsInScreen(listNode);
			if (bounds.left > 0) {
				listNode.recycle();
				performGlobalBack(POLL_DELAY);
				return 0;
			}

			CavanAccessibilityHelper.performScrollUp(listNode, 10);
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
							if (mRetryCount == 0 || CavanAccessibilityHelper.getNodeCountByViewIds(parent, "com.tencent.mobileqq:id/unreadmsg") > 0) {
								mRunnableClick.post(POLL_DELAY_JUMP, false);
								CavanAccessibilityHelper.performClickAndRecycle(parent);
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
				CavanAccessibilityHelper.recycleNodes(chats);
			}

			listNode.recycle();

			return POLL_DELAY;
		}
	}

	@Override
	public String getPackageName() {
		return CavanPackageName.QQ;
	}

	@Override
	protected boolean isValidPackageName(CharSequence pkgName) {
		return CavanPackageName.QQ.equals(pkgName) || CavanPackageName.QWALLET.equals(pkgName);
	}

	@Override
	protected long onPollEventFire(AccessibilityNodeInfo root) {
		CavanAndroid.dLog("mClassName = " + mClassName);

		switch (mClassName) {
		case "com.tencent.mobileqq.activity.SplashActivity":
			return doFindAndUnpack(root);

		case "cooperation.qwallet.plugin.QWalletPluginProxyActivity":
			setUnpackPending(root, false);
			break;

		default:
			if (mClassName.length() > 0) {
				performGlobalBack(POLL_DELAY_JUMP);
			}
			return 0;
		}

		return POLL_DELAY;
	}

	@Override
	public void onWindowStateChanged(AccessibilityEvent event) {
		switch (mClassName) {
		case "com.tencent.mobileqq.activity.SplashActivity":
		case "cooperation.qwallet.plugin.QWalletPluginProxyActivity":
			if (getPacketCount() > 0) {
				setLockEnable(POLL_DELAY, true);

				AccessibilityNodeInfo root = getRootInActiveWindow();
				if (root == null) {
					break;
				}

				if (CavanAccessibilityHelper.containsTextsWhole(root, "群公告")) {
					AccessibilityNodeInfo node = CavanAccessibilityHelper.findNodeByText(root, "我知道了");
					if (node != null) {
						CavanAccessibilityHelper.performClickAndRecycle(node);
						break;
					}

					node = CavanAccessibilityHelper.findNodeByClassName(root, CavanAccessibilityHelper.CLASS_BUTTON);
					if (node != null) {
						CavanAccessibilityHelper.performClickAndRecycle(node);
						break;
					}
				}
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
				RedPacketListenerService listener = RedPacketListenerService.getInstance();
				if (listener != null) {
					listener.addRedPacketContent(source.getPackageName(), text, "QQ消息盒子", true, false, 0);
				}
			}
		}
	}

	@Override
	public void onViewClicked(AccessibilityEvent event) {
		AccessibilityNodeInfo source = event.getSource();
		if (source == null) {
			return;
		}

		if (CavanAccessibilityHelper.isTextView(source)) {
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
				postClickEventMessage(event);
			}
		}
	}

	@Override
	public boolean addPacket(String packet) {
		int delay = CavanMessageActivity.getAutoUnpackQQ(mService);
		if (delay < 0) {
			return false;
		}

		if (super.addPacket(packet)) {
			mRetryCount = 0;
			mChatIndex = 0;
			return true;
		}

		return false;
	}
}
