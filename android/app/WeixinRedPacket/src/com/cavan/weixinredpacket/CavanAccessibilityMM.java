package com.cavan.weixinredpacket;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;

import android.annotation.SuppressLint;
import android.app.Notification;
import android.os.Handler;
import android.os.Message;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.cavan.accessibility.CavanAccessibilityHelper;
import com.cavan.accessibility.CavanAccessibilityPackage;
import com.cavan.accessibility.CavanAccessibilityService;
import com.cavan.accessibility.CavanAccessibilityWindow;
import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;

public class CavanAccessibilityMM extends CavanAccessibilityPackage<CavanNotification> {

	private static final int MSG_SHOW_COUNT_DOWN_DIALOG = 1;
	private static final int MSG_DISMISS_COUNT_DOWN_DIALOG = 2;

	private CountDownDialog mCountDownDialog;
	private WeixinWindow mBaseWindow = new WeixinWindow("BaseWindow");
	private WeixinReceiveWindow mReceiveWindowMore = new WeixinReceiveWindow("ReceiveWindowMore");
	private WeixinChattingWindow mChattingWindowMore = new WeixinChattingWindow("ChattingWindowMore");
	private HashSet<Integer> mFinishNodes = new HashSet<Integer>();

	@SuppressLint("HandlerLeak")
	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case MSG_SHOW_COUNT_DOWN_DIALOG:
				if (mCountDownDialog == null) {
					mCountDownDialog = new CountDownDialog(getService()) {

						@Override
						protected void onButtonCancelClicked() {
							super.onButtonCancelClicked();
							clearPackets();
						}

						@Override
						protected void onButtonNowClicked() {
							super.onButtonNowClicked();

							setUnpackTime(0);
							CavanAccessibilityMM.this.post();
						}
					};
				}

				mCountDownDialog.show(CavanAccessibilityMM.this);
				break;

			case MSG_DISMISS_COUNT_DOWN_DIALOG:
				if (mCountDownDialog != null) {
					mCountDownDialog.dismiss();
					mCountDownDialog = null;
				}
				break;
			}
		}
	};

	public class WeixinWindow extends CavanAccessibilityWindow {

		public WeixinWindow(String name) {
			super(name);
		}

		public boolean isLauncherUi() {
			return false;
		}

		public boolean isWebviewUi() {
			return false;
		}
	}

	public class WeixinChattingWindow extends WeixinWindow {

		protected String mMessageListViewId;
		protected String mRedPacketNodeId;

		public WeixinChattingWindow(String name) {
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

			if (node.getChildCount() != 3) {
				return false;
			}

			List<AccessibilityNodeInfo> childs = CavanAccessibilityHelper.getChilds(node);
			if (childs == null) {
				return false;
			}

			try {
				for (AccessibilityNodeInfo child : childs) {
					if (!TextView.class.getName().equals(child.getClassName())) {
						return false;
					}
				}
			} catch (Exception e) {
				e.printStackTrace();
				return false;
			} finally {
				CavanAccessibilityHelper.recycleNodes(childs);
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
						if ("微信红包".equals(child.getText())) {
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

	public class WeixinLauncherWindow extends WeixinChattingWindow {

		private int mHashCode;

		public WeixinLauncherWindow(String name) {
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

	public class WeixinReceiveWindow extends WeixinWindow {

		private String mUnpackNodeId;
		private String mBackNodeId;

		public WeixinReceiveWindow(String name) {
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
			setPending(true);
		}

		@Override
		public boolean poll(AccessibilityNodeInfo root, int times) {
			long time = System.currentTimeMillis();
			if (mUnpackTime > time) {
				mHandler.sendEmptyMessage(MSG_SHOW_COUNT_DOWN_DIALOG);
				return true;
			}

			mHandler.sendEmptyMessage(MSG_DISMISS_COUNT_DOWN_DIALOG);

			AccessibilityNodeInfo backNode = findBackNode(root);
			if (backNode == null) {
				return false;
			}

			setUnlockDelay(LOCK_DELAY);

			try {
				AccessibilityNodeInfo button = findUnpckNode(root);
				if (button != null) {
					CavanAccessibilityHelper.performClickAndRecycle(button);
				} else {
					CavanAccessibilityHelper.performClick(backNode);
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

	public class WeixinDetailWindow extends WeixinWindow {

		private String mBackNodeId;

		public WeixinDetailWindow(String name) {
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

	public class WeixinWebViewWindow extends WeixinWindow {

		public WeixinWebViewWindow(String name) {
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
		addWindow(new WeixinLauncherWindow("com.tencent.mm.ui.LauncherUI"));
		addWindow(new WeixinChattingWindow("com.tencent.mm.ui.chatting.ChattingUI"));
		addWindow(new WeixinChattingWindow("com.tencent.mm.ui.conversation.BizConversationUI"));
		addWindow(new WeixinReceiveWindow("com.tencent.mm.plugin.luckymoney.ui.LuckyMoneyReceiveUI"));
		addWindow(new WeixinReceiveWindow("com.tencent.mm.plugin.luckymoney.ui.LuckyMoneyBusiReceiveUI"));
		addWindow(new WeixinDetailWindow("com.tencent.mm.plugin.luckymoney.ui.LuckyMoneyDetailUI"));
		addWindow(new WeixinDetailWindow("com.tencent.mm.plugin.luckymoney.ui.LuckyMoneyBusiDetailUI"));
		addWindow(new WeixinWebViewWindow("com.tencent.mm.plugin.webview.ui.tools.WebViewUI"));
	}

	@Override
	public synchronized CavanAccessibilityWindow getWindow(String name) {
		CavanAccessibilityWindow win = super.getWindow(name);
		if (win != null) {
			return win;
		}

		if (name.startsWith("com.tencent.mm.ui.base.")) {
			return mBaseWindow;
		}

		if (name.startsWith("com.tencent.mm.ui.chatting.En_")) {
			return mChattingWindowMore;
		}

		if (name.startsWith("com.tencent.mm.plugin.luckymoney.ui.En_")) {
			return mReceiveWindowMore;
		}

		return null;
	}

	public boolean isWebViewUi() {
		WeixinWindow win = (WeixinWindow) mWindow;
		return (win != null && win.isWebviewUi() && isCurrentPackage());
	}

	@Override
	public String getPackageName() {
		return CavanPackageName.MM;
	}

	@Override
	public long getUnpackDelay() {
		return MainActivity.getAutoUnpackMM(mService) * 1000;
	}

	@Override
	public synchronized boolean addPacket(CavanNotification packet) {
		if (super.addPacket(packet)) {
			mFinishNodes.clear();
			return true;
		}

		return false;
	}

	@Override
	public void onNotificationStateChanged(Notification notification) {
		CavanNotification cn = new CavanNotification(notification);

		if (cn.isRedPacket()) {
			cn.send();
			addPacket(cn);
		}
	}
}
