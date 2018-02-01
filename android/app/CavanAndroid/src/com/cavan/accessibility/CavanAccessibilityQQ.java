package com.cavan.accessibility;

import java.util.HashSet;
import java.util.List;

import android.app.Notification;
import android.graphics.Rect;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;
import com.cavan.java.CavanJava;

public class CavanAccessibilityQQ extends CavanAccessibilityPackage {

	private static final String RED_PACKET_NAME = "QQ红包";
	private static final int SCROLL_DELAY = 200;
	private static final int MAX_SCROLL_COUNT = 2;
	private static final int MAX_RETRY_TIMES = 3;

	public class BaseWindow extends CavanAccessibilityWindow {

		public BaseWindow(String name) {
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
		public boolean poll(AccessibilityNodeInfo root, int times) {
			AccessibilityNodeInfo node = findBackNode(root);
			if (node != null) {
				CavanAccessibilityHelper.performClickAndRecycle(node);
				return true;
			}

			return false;
		}
	}

	public class SplashActivity extends BaseWindow {
		private HashSet<Integer> mFinishNodes = new HashSet<Integer>();
		private int mChatIndex;
		private int mRetryTimes;

		public SplashActivity(String name) {
			super(name);
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
			case "QQ红包":
			case "点击拆开":
			case "QQ红包个性版":
				setUnlockDelay(LOCK_DELAY);
				CavanAccessibilityHelper.performClick(node);
				return true;

			case "口令红包":
				if (!CavanAccessibilityHelper.performClick(node)) {
					break;
				}

				AccessibilityNodeInfo inputBar = CavanAccessibilityHelper.findNodeByViewId(root, "com.tencent.mobileqq:id/inputBar");
				if (inputBar == null) {
					break;
				}

				try {
					if (inputBar.getChildCount() < 2) {
						break;
					}

					if (CavanAccessibilityHelper.setChildText(mService, inputBar, 0, message) == null) {
						break;
					}

					setUnlockDelay(LOCK_DELAY);
					CavanAccessibilityHelper.performChildClick(inputBar, 1);
					return true;
				} catch (Exception e) {
					e.printStackTrace();
				} finally {
					inputBar.recycle();
				}
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

				if (mFinishNodes .contains(hash)) {
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

		private boolean doFindAndUnpack(AccessibilityNodeInfo root) {
			AccessibilityNodeInfo backNode = findBackNode(root);
			if (backNode != null) {
				String title = CavanAccessibilityHelper.getNodeTextByViewId(root, "com.tencent.mobileqq:id/title");
				CavanAndroid.dLog("title = " + title);

				if (title != null) {
					CavanRedPacket packet = findPacket(title);
					CavanAndroid.dLog("packet = " + packet);

					if (packet != null) {
						if (doAutoUnpack(root)) {
							backNode.recycle();
							return true;
						}

						if (removePacket(packet)) {
							CavanAndroid.dLog("complete: " + packet);
						}

						mFinishNodes.clear();
					}
				}

				CavanAccessibilityHelper.performClickAndRecycle(backNode);
				return false;
			} else {
				AccessibilityNodeInfo listNode = CavanAccessibilityHelper.findNodeByViewId(root, "com.tencent.mobileqq:id/recent_chat_list");
				if (listNode == null) {
					CavanAndroid.dLog("List node not found");
					return false;
				}

				mFinishNodes.clear();

				Rect bounds = CavanAccessibilityHelper.getBoundsInScreen(listNode);
				if (bounds.left > 0) {
					CavanAndroid.dLog("Left = " + bounds.left);
					listNode.recycle();
					return false;
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
									CavanAccessibilityHelper.performClickAndRecycle(parent);
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
							clearPackets();
							mRetryTimes = 0;
							break;
						}
					}
				}

				if (chats != null) {
					CavanAccessibilityHelper.recycleNodes(chats);
				}

				listNode.recycle();

				return true;
			}
		}

		@Override
		public boolean poll(AccessibilityNodeInfo root, int times) {
			return doFindAndUnpack(root);
		}
	}

	public class WalletActivity extends BaseWindow {

		public WalletActivity(String name) {
			super(name);
		}

	}

	public CavanAccessibilityQQ(CavanAccessibilityService service) {
		super(service);
	}

	public synchronized CavanRedPacket findPacket(String title) {
		for (CavanRedPacket packet : mPackets) {
			CavanNotificationQQ notification = (CavanNotificationQQ) packet;
			String name = notification.getName();
			if (name == null) {
				continue;
			}

			if (name.contains(title) || title.contains(name)) {
				return packet;
			}
		}

		return null;
	}

	@Override
	public String getPackageName() {
		return CavanPackageName.QQ;
	}

	@Override
	public void initWindows() {
		addWindow(new SplashActivity("com.tencent.mobileqq.activity.SplashActivity"));
		addWindow(new WalletActivity("cooperation.qwallet.plugin.QWalletPluginProxyActivity"));
		addWindow(new BaseWindow("com.tencent.biz.pubaccount.serviceAccountFolder.ServiceAccountFolderActivity"));
		addWindow(new BaseWindow("com.tencent.mobileqq.activity.ChatActivity"));
	}

	@Override
	public synchronized void onNotificationStateChanged(Notification notification) {
		CavanNotificationQQ packet = new CavanNotificationQQ(this, notification);
		if (packet.isRedPacket()) {
			addPacket(packet);
		}
	}

}
