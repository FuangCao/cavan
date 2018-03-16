package com.cavan.accessibility;

import java.util.HashSet;
import java.util.List;

import android.app.Notification;
import android.graphics.Rect;
import android.view.accessibility.AccessibilityNodeInfo;
import android.widget.TextView;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;
import com.cavan.java.CavanJava;

public class CavanAccessibilityQQ extends CavanAccessibilityPackage {

	private static final int SCROLL_DELAY = 200;
	private static final int UNPACK_DELAY = 5000;
	private static final int MAX_SCROLL_COUNT = 2;
	private static final int MAX_RETRY_TIMES = 3;

	public static CavanAccessibilityQQ instance;

	private HashSet<Integer> mFinishNodes = new HashSet<Integer>();
	private int mRetryTimes;
	private int mChatIndex;

	public class BaseWindow extends CavanAccessibilityWindow {

		public BaseWindow(String name) {
			super(name);
		}
	}

	public class BackableWindow extends BaseWindow
	{
		public BackableWindow(String name) {
			super(name);
		}

		public AccessibilityNodeInfo findBackNode(AccessibilityNodeInfo root) {
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

		@Override
		public boolean poll(CavanRedPacket packet, AccessibilityNodeInfo root, int times) {
			AccessibilityNodeInfo node = findBackNode(root);
			if (node != null) {
				return CavanAccessibilityHelper.performClickAndRecycle(node);
			}

			return false;
		}
	}

	public class WalletActivity extends BackableWindow {

		public WalletActivity(String name) {
			super(name);
		}

		public boolean showRedPacketDetail(AccessibilityNodeInfo root) {
			AccessibilityNodeInfo node = CavanAccessibilityHelper.findNodeByText(root, "查看领取详情");
			if (node != null) {
				setUnlockDelay(LOCK_DELAY);
				return CavanAccessibilityHelper.performClickAndRecycle(node);
			}

			return tryCloseDialog(root);
		}

		@Override
		public boolean poll(CavanRedPacket packet, AccessibilityNodeInfo root, int times) {
			if (super.poll(packet, root, times)) {
				return true;
			}

			return showRedPacketDetail(root);
		}
	}

	public class SplashActivity extends BackableWindow {

		private boolean mHomePage;

		public SplashActivity(String name) {
			super(name);
		}

		private boolean doUnpackOnce(AccessibilityNodeInfo root, AccessibilityNodeInfo node) {
			int count = node.getChildCount();
			if (count < 2) {
				return false;
			}

			AccessibilityNodeInfo[] childs = CavanAccessibilityHelper.getChildsRaw(node, 0, count);
			addRecycleNodes(childs);

			for (AccessibilityNodeInfo child : childs) {
				if (!TextView.class.getName().equals(child.getClassName())) {
					return false;
				}
			}

			String state = CavanAccessibilityHelper.getNodeText(childs[1]);
			if (state == null) {
				return false;
			}

			if (count > 2) {
				String type = CavanAccessibilityHelper.getNodeText(childs[2]);
				if (type == null) {
					return false;
				}

				CavanAndroid.dLog("type = " + type);

				if (!type.equals("QQ红包")) {
					return false;
				}
			} else if (state.startsWith("QQ") && state.endsWith("红包")) {
				int length = state.length();
				if (length > 4) {
					state = state.substring(2, length - 2);
				}
			} else if (state.startsWith("QQ红包")) {
				state = "QQ红包";
			} else {
				return false;
			}

			String message = CavanAccessibilityHelper.getNodeText(childs[0]);
			CavanAndroid.dLog("message = " + message);

			if (message == null || message.contains("测") || message.contains("挂")) {
				return false;
			}

			CavanAndroid.dLog("state = " + state);

			switch (state) {
			case "会员":
			case "QQ红包":
			case "点击拆开":
			case "QQ红包个性版":
				if (!CavanAccessibilityHelper.performClick(node)) {
					return false;
				}
				break;

			case "口令红包":
			case "文字口令":
				if (!sendText(root, message, true)) {
					return false;
				}
				break;

			default:
				CavanAndroid.dLog("Invalid state: " + state);
				return false;
			}

			setUnlockDelay(UNPACK_DELAY);

			return true;
		}

		private boolean doAutoUnpack(AccessibilityNodeInfo root, AccessibilityNodeInfo listNode) {
			List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByViewId("com.tencent.mobileqq:id/chat_item_content_layout");
			if (nodes == null) {
				return false;
			}

			addRecycleNodes(nodes);

			int index;

			for (index = nodes.size() - 1; index >= 0; index--) {
				AccessibilityNodeInfo node = nodes.get(index);
				int hash = node.hashCode();

				if (mFinishNodes .contains(hash)) {
					continue;
				}

				if (doUnpackOnce(root, node)) {
					mFinishNodes.add(hash);
					return true;
				}
			}

			return false;
		}

		private boolean doAutoUnpack(AccessibilityNodeInfo root) {
			AccessibilityNodeInfo listNode = CavanAccessibilityHelper.findNodeByViewId(root, "com.tencent.mobileqq:id/listView1");
			if (listNode == null) {
				listNode = CavanAccessibilityHelper.findChildByClassName(root, CavanAccessibilityHelper.CLASS_ABS_LIST_VIEW);
				if (listNode == null) {
					return false;
				}
			}

			try {
				if (mFinishNodes.isEmpty() && CavanAccessibilityHelper.performScrollDown(listNode, 100)) {
					CavanJava.msleep(SCROLL_DELAY);
				}

				for (int i = 0; i < MAX_SCROLL_COUNT; i++) {
					if (doAutoUnpack(root, listNode)) {
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
						return true;
					}
				}
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				listNode.recycle();
			}

			return false;
		}

		private boolean doFindSession(AccessibilityNodeInfo listNode) {
			mFinishNodes.clear();

			Rect bounds = CavanAccessibilityHelper.getBoundsInScreen(listNode);
			if (bounds.left > 0) {
				CavanAndroid.dLog("Left = " + bounds.left);
				mService.performActionBack();
				return true;
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
							if (mRetryTimes == 0 || CavanAccessibilityHelper.getNodeCountByViewIds(parent, "com.tencent.mobileqq:id/unreadmsg") > 0) {
								setUnlockDelay(POLL_DELAY);
								CavanAccessibilityHelper.performClickAndRecycle(parent);
								resetTimes();
								break;
							}
						}

						parent.recycle();
					}
				} else {
					CavanAndroid.dLog("mRetryCount = " + mPollTimes);

					mChatIndex = 0;

					if (++mRetryTimes > MAX_RETRY_TIMES) {
						CavanAndroid.dLog("No packet found");
						removePackets();
						mRetryTimes = 0;
						break;
					}
				}
			}

			if (chats != null) {
				CavanAccessibilityHelper.recycleNodes(chats);
			}

			return true;
		}

		private boolean doFindAndUnpack(AccessibilityNodeInfo root, AccessibilityNodeInfo backNode) {
			String title = CavanAccessibilityHelper.getNodeTextByViewId(root, "com.tencent.mobileqq:id/title");
			CavanAndroid.dLog("title = " + title);

			if (title != null) {
				CavanRedPacket packet = findPacket(title);
				CavanAndroid.dLog("packet = " + packet);

				if (packet != null && packet.isPending()) {
					if (doAutoUnpack(root)) {
						return true;
					}

					packet.setCompleted();
					mFinishNodes.clear();
				}
			}

			return CavanAccessibilityHelper.performClick(backNode);
		}

		@Override
		public boolean poll(CavanRedPacket packet, AccessibilityNodeInfo root, int times) {
			AccessibilityNodeInfo backNode = findBackNode(root);
			if (backNode != null) {
				mHomePage = false;

				try {
					return doFindAndUnpack(root, backNode);
				} catch (Exception e) {
					e.printStackTrace();
				} finally {
					backNode.recycle();
				}

				return false;
			}

			AccessibilityNodeInfo listNode = CavanAccessibilityHelper.findNodeByViewId(root, "com.tencent.mobileqq:id/recent_chat_list");
			if (listNode != null) {
				mHomePage = true;

				if (packet.isCompleted()) {
					return true;
				}

				try {
					return doFindSession(listNode);
				} catch (Exception e) {
					e.printStackTrace();
				} finally {
					listNode.recycle();
				}

				return false;
			}

			mHomePage = false;

			if (tryCloseDialog(root)) {
				return true;
			}

			if (times < 5) {
				return true;
			}

			mService.performActionBack();
			return false;
		}

		@Override
		public boolean isHomePage() {
			return mHomePage;
		}

		@Override
		protected boolean doSendText(AccessibilityNodeInfo root, String message, boolean commit) {
			return sendText(root, message, commit);
		}
	}

	public CavanAccessibilityQQ(CavanAccessibilityService service) {
		super(service, new String[] { CavanPackageName.QQ, CavanPackageName.QWALLET });
	}

	public synchronized CavanRedPacket findPacket(String title) {
		CavanRedPacketList packets = getPackets();

		synchronized (packets) {
			for (CavanRedPacket packet : packets) {
				if (packet.getPackage() != this) {
					continue;
				}

				CavanNotificationQQ notification = (CavanNotificationQQ) packet;
				String name = notification.getName();
				if (name == null) {
					continue;
				}

				if (name.contains(title) || title.contains(name)) {
					return packet;
				}
			}
		}

		return null;
	}

	public boolean sendText(AccessibilityNodeInfo root, String message, boolean commit) {
		AccessibilityNodeInfo inputBar = CavanAccessibilityHelper.findNodeByViewId(root, "com.tencent.mobileqq:id/inputBar");
		if (inputBar == null) {
			return false;
		}

		AccessibilityNodeInfo[] childs = null;

		try {
			childs = CavanAccessibilityHelper.getChilds(inputBar, 2);
			if (childs == null) {
				return false;
			}

			if (message != null && CavanAccessibilityHelper.setChildText(mService, inputBar, 0, message) == null) {
				return false;
			}

			if (commit) {
				return CavanAccessibilityHelper.performChildClick(inputBar, 1);
			}

			return true;
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			if (childs != null) {
				CavanAccessibilityHelper.recycleNodes(childs);
			}

			inputBar.recycle();
		}

		return false;
	}

	@Override
	public void initWindows() {
		addProgressWindow("com.tenpay.sdk.h.au");
		addProgressWindow("com.tencent.mobileqq.activity.PayBridgeActivity");
		addWindow(new SplashActivity("com.tencent.mobileqq.activity.SplashActivity"));
		addWindow(new WalletActivity("cooperation.qwallet.plugin.QWalletPluginProxyActivity"));
		addWindow(new BackableWindow("com.tencent.biz.pubaccount.serviceAccountFolder.ServiceAccountFolderActivity"));
		addWindow(new BackableWindow("com.tencent.mobileqq.activity.ChatActivity"));
	}

	@Override
	public void onPacketAdded(CavanRedPacket packet) {
		mFinishNodes.clear();
		mRetryTimes = 0;
		mChatIndex = 0;
	}

	@Override
	public void onNotificationStateChanged(Notification notification) {
		CavanNotificationQQ packet = new CavanNotificationQQ(notification);
		if (packet.isRedPacket()) {
			addPacket(packet);
		}
	}

	@Override
	protected void onCreate() {
		super.onCreate();
		instance = this;
	}

	@Override
	protected void onDestroy() {
		instance = null;
		super.onDestroy();
	}

}
