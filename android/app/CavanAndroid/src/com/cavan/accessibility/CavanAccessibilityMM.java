package com.cavan.accessibility;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;

import android.app.Notification;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;
import com.cavan.java.CavanString;

public class CavanAccessibilityMM extends CavanAccessibilityPackage {

	private HashSet<Integer> mFinishNodes = new HashSet<Integer>();
	private boolean mUnpackSuccess;

	public class BaseWindow extends CavanAccessibilityWindow {

		public BaseWindow(String name) {
			super(name);
		}

		public boolean isLauncherUi() {
			return false;
		}

		public boolean isWebviewUi() {
			return false;
		}
	}

	public class ChattingWindow extends BaseWindow {

		protected String mMessageListViewId;
		protected String mRedPacketNodeId;

		public ChattingWindow(String name) {
			super(name);
		}

		private boolean isValidMessage(AccessibilityNodeInfo node) {
			if (node.getChildCount() != 3) {
				return false;
			}

			String message = CavanAccessibilityHelper.getChildText(node, 0);
			CavanAndroid.dLog("message = " + message);

			if (message == null || message.contains("测") || message.contains("挂")) {
				return false;
			}

			String action = CavanAccessibilityHelper.getChildText(node, 1);
			CavanAndroid.dLog("action = " + action);

			return "领取红包".equals(action);
		}

		private boolean isRedPacketNode(AccessibilityNodeInfo node) {
			if (!LinearLayout.class.getName().equals(node.getClassName())) {
				return false;
			}

			AccessibilityNodeInfo[] childs = CavanAccessibilityHelper.getChilds(node, 0, 3);
			if (childs == null) {
				return false;
			}

			addRecycleNodes(childs);

			for (AccessibilityNodeInfo child : childs) {
				if (!TextView.class.getName().equals(child.getClassName())) {
					return false;
				}
			}

			return true;
		}

		private AccessibilityNodeInfo findMessageListViewRaw(AccessibilityNodeInfo root) {
			List<AccessibilityNodeInfo> nodes = new ArrayList<AccessibilityNodeInfo>();

			try {
				AccessibilityNodeInfo node = root;

				while (node.getChildCount() > 0) {
					node = node.getChild(0);
					if (node == null) {
						break;
					}

					nodes.add(node);

					if (LinearLayout.class.getName().equals(node.getClassName())) {
						return CavanAccessibilityHelper.findChildByClassName(node, ListView.class.getName());
					}
				}
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				CavanAccessibilityHelper.recycleNodes(nodes);
			}

			return null;
		}

		private synchronized AccessibilityNodeInfo findMessageListView(AccessibilityNodeInfo root) {
			if (mMessageListViewId != null) {
				return CavanAccessibilityHelper.findNodeByViewId(root, mMessageListViewId);
			}

			AccessibilityNodeInfo node = findMessageListViewRaw(root);
			if (node == null) {
				return null;
			}

			mMessageListViewId = node.getViewIdResourceName();
			return node;
		}

		private synchronized List<AccessibilityNodeInfo> findRedPacketNodes(AccessibilityNodeInfo root) {
			List<AccessibilityNodeInfo> nodes = new ArrayList<AccessibilityNodeInfo>();

			if (mRedPacketNodeId != null) {
				List<AccessibilityNodeInfo> childs = root.findAccessibilityNodeInfosByViewId(mRedPacketNodeId);
				if (childs != null) {
					for (AccessibilityNodeInfo child : childs) {
						String text = CavanString.fromCharSequence(child.getText(), null);
						if (text != null && "微信红包".equals(text)) {
							AccessibilityNodeInfo node = child.getParent();
							if (node != null) {
								if (LinearLayout.class.getName().equals(node.getClassName())) {
									nodes.add(node);
								} else {
									node.recycle();
								}
							}
						}

						child.recycle();
					}
				}

				return nodes;
			}

			AccessibilityNodeInfo listView = findMessageListView(root);
			if (listView == null) {
				CavanAndroid.eLog("Failed to findMessageListView");
				return nodes;
			}

			List<AccessibilityNodeInfo> childs = CavanAccessibilityHelper.findNodesByText(listView, "微信红包");
			if (childs != null) {
				for (AccessibilityNodeInfo child : childs) {
					AccessibilityNodeInfo node = child.getParent();
					if (node != null) {
						if (isRedPacketNode(node)) {
							mRedPacketNodeId = child.getViewIdResourceName();
							CavanAndroid.dLog("mRedPacketNodeId = " + mRedPacketNodeId);
							nodes.add(node);
						} else {
							node.recycle();
						}
					}

					child.recycle();
				}
			}

			listView.recycle();

			return nodes;
		}

		private synchronized boolean doFindAndUnpack(AccessibilityNodeInfo root) {
			List<AccessibilityNodeInfo> nodes = findRedPacketNodes(root);
			if (nodes == null || nodes.isEmpty()) {
				return false;
			}

			try {
				for (int i = nodes.size() - 1; i >= 0; i--) {
					AccessibilityNodeInfo node = nodes.get(i);
					int hash = node.hashCode();

					if (mFinishNodes.contains(hash)) {
						continue;
					}

					if (isValidMessage(node)) {
						mFinishNodes.add(hash);
						setUnlockDelay(LOCK_DELAY);
						CavanAccessibilityHelper.performClick(node);
						return true;
					}
				}
			} catch (Exception e) {
				e.printStackTrace();
				return false;
			} finally {
				CavanAccessibilityHelper.recycleNodes(nodes);
			}

			clearPackets();

			return false;
		}

		@Override
		public boolean isLauncherUi() {
			return true;
		}

		@Override
		public synchronized void onPackageUpdated() {
			mMessageListViewId = null;
			mRedPacketNodeId = null;
		}

		@Override
		public boolean poll(AccessibilityNodeInfo root, int times) {
			return doFindAndUnpack(root);
		}

		@Override
		public boolean onPollFailed(int times) {
			if (super.onPollFailed(times)) {
				return true;
			}

			clearPackets();

			return false;
		}
	}

	public class LauncherWindow extends ChattingWindow {

		private int mHashCode;

		public LauncherWindow(String name) {
			super(name);
		}

		public boolean isRedPacketLayout(AccessibilityNodeInfo node) {
			if (RelativeLayout.class.getName().equals(node.getClassName())) {
				AccessibilityNodeInfo parent = node.getParent();
				if (parent == null) {
					return false;
				}

				try {
					if (ListView.class.getName().equals(parent.getClassName())) {
						return (CavanAccessibilityHelper.getNodeCountByTexts(node, "微信红包") > 0);
					}
				} catch (Exception e) {
					e.printStackTrace();
				} finally {
					parent.recycle();
				}
			}

			return false;
		}

		@Override
		public void onEnter() {
			AccessibilityNodeInfo root = getRootInActiveWindow();
			if (root != null) {
				int hashCode = root.hashCode();
				if (hashCode != mHashCode) {
					mHashCode = hashCode;
					CavanAndroid.dLog("mHashCode = " + Integer.toHexString(hashCode));
					performPackageUpdated();
				}
			}
		}

		@Override
		public void onWindowContentChanged(AccessibilityEvent event) {
			AccessibilityNodeInfo source = event.getSource();
			if (source != null) {
				if (isRedPacketLayout(source)) {
					setUnlockDelay(200);
					addPacket(null);
				}

				source.recycle();
			}
		}
	}

	public class ReceiveWindow extends BaseWindow {

		private String mUnpackNodeId;
		private String mBackNodeId;

		public ReceiveWindow(String name) {
			super(name);
		}

		private AccessibilityNodeInfo findBackNodeRaw(AccessibilityNodeInfo root) {
			if (root.getChildCount() > 4) {
				AccessibilityNodeInfo node = root.getChild(4);
				if (node != null) {
					if (ImageView.class.getName().equals(node.getClassName())) {
						return node;
					}

					node.recycle();
				}
			}

			return CavanAccessibilityHelper.findChildByClassName(root, ImageView.class.getName());
		}

		private AccessibilityNodeInfo findBackNode(AccessibilityNodeInfo root) {
			if (mBackNodeId != null) {
				return CavanAccessibilityHelper.findNodeByViewId(root, mBackNodeId);
			}

			AccessibilityNodeInfo node = findBackNodeRaw(root);
			if (node == null) {
				return null;
			}

			mBackNodeId = node.getViewIdResourceName();
			CavanAndroid.dLog("mBackNodeId = " + mBackNodeId);

			return node;
		}

		private AccessibilityNodeInfo findUnpckNodeRaw(AccessibilityNodeInfo root) {
			if (root.getChildCount() > 3) {
				AccessibilityNodeInfo node = root.getChild(3);
				if (node != null) {
					if (Button.class.getName().equals(node.getClassName())) {
						return node;
					}

					node.recycle();
				}
			}

			return CavanAccessibilityHelper.findChildByClassName(root, Button.class.getName());
		}

		private AccessibilityNodeInfo findUnpckNode(AccessibilityNodeInfo root) {
			if (mUnpackNodeId != null) {
				return CavanAccessibilityHelper.findNodeByViewId(root, mUnpackNodeId);
			}

			AccessibilityNodeInfo node = findUnpckNodeRaw(root);
			if (node == null) {
				return null;
			}

			mUnpackNodeId = node.getViewIdResourceName();
			CavanAndroid.dLog("mUnpackNodeId = " + mUnpackNodeId);

			return node;
		}

		@Override
		public void onPackageUpdated() {
			mUnpackNodeId = null;
			mBackNodeId = null;
		}

		@Override
		public void onEnter() {
			if (isForceUnpackEnabled()) {
				setPending(true);
			}

			mUnpackSuccess = false;
		}

		@Override
		public void onLeave() {
			setForceUnpackEnable(true);
		}

		@Override
		public boolean poll(AccessibilityNodeInfo root, int times) {
			long time = System.currentTimeMillis();
			if (mUnpackTime > time) {
				showCountDownView();
				return true;
			}

			dismissCountDownView();

			AccessibilityNodeInfo backNode = findBackNode(root);
			if (backNode == null) {
				return false;
			}

			setUnlockDelay(LOCK_DELAY);

			try {
				AccessibilityNodeInfo button = findUnpckNode(root);
				if (button != null) {
					CavanAccessibilityHelper.performClickAndRecycle(button);
					mUnpackSuccess = true;
				} else if (getPacketCount() > 0) {
					CavanAccessibilityHelper.performClick(backNode);
				} else {
					setPending(false);
				}
			} catch (Exception e) {
				e.printStackTrace();
				return false;
			} finally {
				backNode.recycle();
			}

			return true;
		}
	}

	public class DetailWindow extends BaseWindow {

		private String mBackNodeId;

		public DetailWindow(String name) {
			super(name);
		}

		private AccessibilityNodeInfo findBackNodeRaw(AccessibilityNodeInfo root) {
			AccessibilityNodeInfo node = CavanAccessibilityHelper.findNodeByClassName(root, ImageView.class.getName());
			if (node == null) {
				return null;
			}

			AccessibilityNodeInfo parent = node.getParent();
			if (parent != null && LinearLayout.class.getName().equals(parent.getClassName())) {
				node.recycle();
				return parent;
			}

			return node;
		}

		private AccessibilityNodeInfo findBackNode(AccessibilityNodeInfo root) {
			if (mBackNodeId != null) {
				return CavanAccessibilityHelper.findNodeByViewId(root, mBackNodeId);
			}

			AccessibilityNodeInfo node = findBackNodeRaw(root);
			if (node == null) {
				return null;
			}

			mBackNodeId = node.getViewIdResourceName();
			CavanAndroid.dLog("mBackNodeId = " + mBackNodeId);

			return node;
		}

		@Override
		public void onPackageUpdated() {
			mBackNodeId = null;
		}

		@Override
		public void onEnter() {
			CavanAndroid.dLog("mUnpackSuccess = " + mUnpackSuccess);
			setPending(mUnpackSuccess);
		}

		@Override
		public boolean poll(AccessibilityNodeInfo root, int times) {
			AccessibilityNodeInfo backNode = findBackNode(root);
			if (backNode == null) {
				return false;
			}

			setUnlockDelay(LOCK_DELAY);
			CavanAccessibilityHelper.performClickAndRecycle(backNode);

			return true;
		}
	}

	public class WebViewWindow extends BaseWindow {

		public WebViewWindow(String name) {
			super(name);
		}

		@Override
		public boolean isWebviewUi() {
			return isCurrentPackage();
		}

		@Override
		public boolean poll(AccessibilityNodeInfo root, int times) {
			return false;
		}
	}

	public CavanAccessibilityMM(CavanAccessibilityService service) {
		super(service);
	}

	public boolean isWebViewUi() {
		BaseWindow win = (BaseWindow) mWindow;
		return (win != null && win.isWebviewUi() && isCurrentPackage());
	}

	public BaseWindow getBaseWindow(String name) {
		return new BaseWindow(name);
	}

	public ReceiveWindow getReceiveWindow(String name) {
		return new ReceiveWindow(name);
	}

	public ChattingWindow getChattingWindow(String name) {
		return new ChattingWindow(name);
	}

	public CavanAccessibilityWindow getWebViewWindow(String name) {
		return new WebViewWindow(name);
	}

	public CavanAccessibilityWindow getDetailWindow(String name) {
		return new DetailWindow(name);
	}

	public CavanAccessibilityWindow getLauncherWindow(String name) {
		return new LauncherWindow(name);
	}

	@Override
	public synchronized CavanAccessibilityWindow getWindow(String name) {
		CavanAccessibilityWindow win = super.getWindow(name);
		if (win != null) {
			return win;
		}

		if (name.startsWith("com.tencent.mm.ui.base.")) {
			win = new BaseWindow(name);
		} else if (name.startsWith("com.tencent.mm.ui.chatting.En_")) {
			win = new ChattingWindow(name);
		} else if (name.startsWith("com.tencent.mm.plugin.luckymoney.ui.En_")) {
			win = new ReceiveWindow(name);
		} else {
			return null;
		}

		addWindow(win);

		return win;
	}

	@Override
	public void initWindows() {
		addWindow(getLauncherWindow("com.tencent.mm.ui.LauncherUI"));
		addWindow(getChattingWindow("com.tencent.mm.ui.chatting.ChattingUI"));
		addWindow(getChattingWindow("com.tencent.mm.ui.conversation.BizConversationUI"));
		addWindow(getReceiveWindow("com.tencent.mm.plugin.luckymoney.ui.LuckyMoneyReceiveUI"));
		addWindow(getReceiveWindow("com.tencent.mm.plugin.luckymoney.ui.LuckyMoneyBusiReceiveUI"));
		addWindow(getDetailWindow("com.tencent.mm.plugin.luckymoney.ui.LuckyMoneyDetailUI"));
		addWindow(getDetailWindow("com.tencent.mm.plugin.luckymoney.ui.LuckyMoneyBusiDetailUI"));
		addWindow(getWebViewWindow("com.tencent.mm.plugin.webview.ui.tools.WebViewUI"));
	}

	@Override
	public String getPackageName() {
		return CavanPackageName.MM;
	}

	@Override
	public void onPacketAdded(CavanRedPacket packet) {
		mFinishNodes.clear();
	}

	@Override
	public void onNotificationStateChanged(Notification notification) {
		CavanNotificationMM packet = new CavanNotificationMM(this, notification);
		if (packet.isRedPacket()) {
			addPacket(packet);
		}
	}

}
