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

	public static CavanAccessibilityMM instance;

	public class BaseWindow extends CavanAccessibilityWindow {

		public BaseWindow(String name) {
			super(name);
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

		public AccessibilityNodeInfo findInputNode(AccessibilityNodeInfo root) {
			AccessibilityNodeInfo node = root.findFocus(AccessibilityNodeInfo.FOCUS_INPUT);
			CavanAndroid.pLog("node = " + node);
			if (node != null) {
				if (CavanAccessibilityHelper.isEditText(node)) {
					return node;
				}

				node.recycle();
			}

			node = CavanAccessibilityHelper.getChildRecursive(root, 0, 0, 0, -1, 0, 1, 0);
			CavanAndroid.pLog("node = " + node);
			if (node != null) {
				if (CavanAccessibilityHelper.isEditText(node)) {
					return node;
				}

				node.recycle();
			}

			return null;
		}

		public boolean doCommitMessage(AccessibilityNodeInfo root) {
			List<AccessibilityNodeInfo> nodes = CavanAccessibilityHelper.findNodesByText(root, "发送");
			if (nodes == null) {
				return false;
			}

			try {
				for (AccessibilityNodeInfo node : nodes) {
					if (CavanAccessibilityHelper.isButton(node)) {
						return CavanAccessibilityHelper.performClick(node);
					}
				}
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				CavanAccessibilityHelper.recycleNodes(nodes);
			}

			return false;
		}

		@Override
		protected boolean doSendText(AccessibilityNodeInfo root, String message, boolean commit) {
			if (message != null) {
				AccessibilityNodeInfo node = findInputNode(root);
				if (node == null) {
					return false;
				}

				try {
					if (CavanAccessibilityHelper.setNodeText(mService, node, message) == null) {
						return false;
					}
				} catch (Exception e) {
					e.printStackTrace();
				} finally {
					node.recycle();
				}
			}

			return doCommitMessage(root);
		}

		@Override
		public synchronized void onPackageUpdated() {
			mMessageListViewId = null;
			mRedPacketNodeId = null;
		}

		@Override
		public boolean poll(CavanRedPacket packet, AccessibilityNodeInfo root, int times) {
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
			} finally {
				CavanAccessibilityHelper.recycleNodes(nodes);
			}

			packet.setCompleted();

			return true;
		}

		@Override
		public boolean isMainActivity() {
			return true;
		}

		@Override
		public void onLeave() {
			mUnpackSuccess = false;
		}
	}

	public class LauncherWindow extends ChattingWindow {

		private int mHashCode;
		private List<AccessibilityNodeInfo> mChangedNodes = new ArrayList<AccessibilityNodeInfo>();

		public LauncherWindow(String name) {
			super(name);
		}

		public boolean isRedPacketLayout(AccessibilityNodeInfo node) {
			AccessibilityNodeInfo parent = node.getParent();
			if (parent == null) {
				return false;
			}

			try {
				if (CavanAccessibilityHelper.isNodeClassEquals(parent, ListView.class)) {
					return (CavanAccessibilityHelper.getNodeCountByTexts(node, "微信红包") > 0);
				}
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				parent.recycle();
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
				if (CavanAccessibilityHelper.isNodeClassEquals(source, RelativeLayout.class)) {
					synchronized (mChangedNodes) {
						mChangedNodes.add(source);
					}

					startWaitReady();
				} else {
					source.recycle();
				}
			}
		}

		@Override
		public boolean onWindowContentReady(int times) {
			CavanAndroid.dLog("onWindowContentReady: " + times);

			synchronized (mChangedNodes) {
				for (AccessibilityNodeInfo node : mChangedNodes) {
					if (isRedPacketLayout(node)) {
						setPending(true);
						break;
					}
				}

				CavanAccessibilityHelper.recycleNodes(mChangedNodes);
				mChangedNodes.clear();
			}

			return true;
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
		}

		@Override
		public void onLeave() {
			setForceUnpackEnable(true);
		}

		@Override
		public boolean poll(CavanRedPacket packet, AccessibilityNodeInfo root, int times) {
			long time = System.currentTimeMillis();
			if (packet.getUnpackTime() > time) {
				showCountDownView(packet);
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
				} else if (getCurrentPacket() != null) {
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
			CavanRedPacket packet = getCurrentPacket();
			CavanAndroid.dLog("packet = " + packet);
			if (getCurrentPacket() == null) {
				CavanAndroid.dLog("mUnpackSuccess = " + mUnpackSuccess);
				setPending(mUnpackSuccess);
			}
		}

		@Override
		public void onLeave() {
			mUnpackSuccess = false;
		}

		@Override
		public boolean poll(CavanRedPacket packet, AccessibilityNodeInfo root, int times) {
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
		public boolean poll(CavanRedPacket packet, AccessibilityNodeInfo root, int times) {
			return false;
		}
	}

	public class ProgressWindow extends BaseWindow {

		public ProgressWindow(String name) {
			super(name);
		}

		@Override
		public boolean isProgressView() {
			return true;
		}
	}

	public CavanAccessibilityMM(CavanAccessibilityService service) {
		super(service, CavanPackageName.MM);
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
			win = new ProgressWindow(name);
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
	public void onPollStarted() {
		mFinishNodes.clear();
	}

	@Override
	public void onNotificationStateChanged(Notification notification) {
		CavanNotificationMM packet = new CavanNotificationMM(notification);
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