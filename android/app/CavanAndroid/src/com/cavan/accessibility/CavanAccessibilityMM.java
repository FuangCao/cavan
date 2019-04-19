package com.cavan.accessibility;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;

import android.app.Notification;
import android.view.KeyEvent;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;
import android.webkit.WebView;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;
import com.cavan.java.CavanJava;
import com.cavan.java.CavanString;

public class CavanAccessibilityMM extends CavanAccessibilityPackage {

	private HashMap<String, String> mSubjects = new HashMap<String, String>();
	private HashSet<Integer> mFinishNodes = new HashSet<Integer>();
	private AppBrandSubject mSubject;
	private String mMenuItem;
	private boolean mAutoAnswer;
	private long mAnswerTime;
	private int mAnswerTimes;
	private BrandServiceIndexWindow mBrandServiceIndexWindow;

	public static CavanAccessibilityMM instance;

	private Runnable mRunnableAutoAnswer = new Runnable() {

		@Override
		public void run() {
			if (mAutoAnswer) {
				long time = System.currentTimeMillis();
				if (time < mAnswerTime) {
					postDelayed(this, mAnswerTime - time);
				} else if (mAnswerTimes < 3) {
					boolean pending = doAutoAnswer();
					CavanAndroid.dLog("doAutoAnswer: times = " + mAnswerTimes + ", pending = " + pending);

					if (pending) {
						postAutoAnswer(200, 0);
					} else {
						postAutoAnswer(200, mAnswerTimes + 1);
					}
				}
			} else {
				doAutoAnswer();
			}
		}
	};

	public class BaseWindow extends CavanAccessibilityWindow {

		public BaseWindow(String name) {
			super(name);
		}

		public boolean isWebviewUi() {
			return false;
		}
	}

	public class MenuWindow extends BaseWindow {

		public MenuWindow(String name) {
			super(name);
		}

		public boolean doClickMenuItem(AccessibilityNodeInfo root, String item) {
			return false;
		}

		@Override
		public boolean isPopWindow() {
			return true;
		}

		@Override
		protected void onEnter(AccessibilityNodeInfo root) {
			if (mMenuItem != null) {
				CavanAndroid.dLog("mMenuItem = " + mMenuItem);
				doClickMenuItem(root, mMenuItem);
				mMenuItem = null;
			}
		}
	}

	public class ChattingMenu extends MenuWindow {

		public ChattingMenu(String name) {
			super(name);
		}

		@Override
		public boolean doClickMenuItem(AccessibilityNodeInfo root, String item) {
			AccessibilityNodeInfo node = CavanAccessibilityHelper.findNodeByText(root, item);
			if (node == null) {
				return false;
			}

			return CavanAccessibilityHelper.performClickAndRecycle(node);
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

			return "领取红包".equals(action) || "微信红包".equals(action);
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
			if (node != null) {
				if (CavanAccessibilityHelper.isEditText(node)) {
					return node;
				}

				node.recycle();
			}

			node = CavanAccessibilityHelper.getChildRecursiveF(root, 0, 0, 0, -1, 0, 1, 0);
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

		public String getChattingName(AccessibilityNodeInfo root) {
			AccessibilityNodeInfo node = CavanAccessibilityHelper.getChildRecursiveF(root, 0, -2);
			if (node == null) {
				return null;
			}

			try {
				if (CavanAccessibilityHelper.isTextView(node)) {
					return CavanAccessibilityHelper.getNodeText(node);
				}
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				node.recycle();
			}

			return null;
		}

		public boolean doClickMenuItem(AccessibilityNodeInfo root, String menu, String item) {
			List<AccessibilityNodeInfo> nodes = CavanAccessibilityHelper.findNodesByText(root, menu);
			if (nodes == null || nodes.isEmpty()) {
				return false;
			}

			try {
				AccessibilityNodeInfo node = nodes.get(nodes.size() - 1);
				if (CavanAccessibilityHelper.performClickParent(node)) {
					mMenuItem = item;
					return true;
				}
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				CavanAccessibilityHelper.recycleNodes(nodes);
			}

			return false;
		}

		@Override
		protected boolean doSignin(AccessibilityNodeInfo root) {
			setSigninPending(false);

			String chatting = getChattingName(root);
			if (chatting == null) {
				return false;
			}

			switch (chatting) {
			case "轩辕传奇手游":
				doClickMenuItem(root, "活动专区", "CDKey兑换");
				break;

			case "征途手机版":
				doClickMenuItem(root, "互动专区", "礼包兑换");
				break;

			case "热血传奇手游":
				doClickMenuItem(root, "我的互动", "礼包兑换");
				break;

			case "新剑侠情缘手游":
				doClickMenuItem(root, "重磅福利", "兑换中心");
				break;

			case "传奇霸业手游":
				doClickMenuItem(root, "公测豪礼", "CDK兑换");
				break;

			case "绝地求生全军出击":
				doClickMenuItem(root, "福利补给", "CDKEY兑换");
				break;

			case "择天记手游":
				doClickMenuItem(root, "神都福利", "CDK兑换");
				break;

			case "QQ飞车手游":
				doClickMenuItem(root, "橘子有礼", "CDK兑换");
				break;

			case "穿越火线官方FPS手游":
				return doClickMenuItem(root, "官网", "CDK兑换");
			}

			return false;
		}

		@Override
		protected boolean doUnfollow(AccessibilityNodeInfo root) {
			/* AccessibilityNodeInfo node = CavanAccessibilityHelper.getChildRecursiveF(root, 0, -1);
			if (node == null) {
				return false;
			}

			try {
				if ("聊天信息".equals(CavanAccessibilityHelper.getNodeDescription(node))) {
					return CavanAccessibilityHelper.performClick(node);
				}
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				node.recycle();
			}

			return false; */

			return doActionBack(root);
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
		protected boolean doActionBack(AccessibilityNodeInfo root) {
			return doActionBackBase(root, 0, 1, 0);
		}

		@Override
		protected boolean doActionHome(AccessibilityNodeInfo root) {
			return doActionBack(root);
		}

		@Override
		protected void onEnter(AccessibilityNodeInfo root) {
			setForceUnpackEnable(true);
			setFollowPending(false);

			if (isSigninPending()) {
				doSignin(root);
			}
		}

		@Override
		public boolean isHomePage() {
			return true;
		}
	}

	public class LauncherWindow extends ChattingWindow {

		private int mHashCode;
		private List<AccessibilityNodeInfo> mChangedNodes = new ArrayList<AccessibilityNodeInfo>();

		public LauncherWindow(String name) {
			super(name);
		}

		@Override
		public String getChattingName(AccessibilityNodeInfo root) {
			AccessibilityNodeInfo node = CavanAccessibilityHelper.getChildRecursiveF(root, 0, 0, 0, 1);
			if (node == null) {
				return null;
			}

			try {
				if (CavanAccessibilityHelper.isTextView(node)) {
					return CavanAccessibilityHelper.getNodeText(node);
				}
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				node.recycle();
			}

			return null;
		}

		public AccessibilityNodeInfo getChattingListView(AccessibilityNodeInfo root) {
			AccessibilityNodeInfo[] nodes = CavanAccessibilityHelper.getChildsRecursiveF(root, 0, 0, 0);
			if (nodes == null) {
				return null;
			}

			if (isWxViewPager(nodes[1]) && CavanAccessibilityHelper.isListView(nodes[2])) {
				CavanAccessibilityHelper.recycleNodes(nodes, 2);
				return nodes[2];
			}

			CavanAccessibilityHelper.recycleNodes(nodes);

			return null;
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

		public boolean clickOfficialAccounts(AccessibilityNodeInfo root) {
			List<AccessibilityNodeInfo> nodes = CavanAccessibilityHelper.findNodesByTexts(root, "公众号名片");
			if (nodes == null || nodes.isEmpty()) {
				return false;
			}

			try {
				return CavanAccessibilityHelper.performClickParent(nodes.get(nodes.size() - 1));
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				CavanAccessibilityHelper.recycleNodes(nodes);
			}

			return false;
		}

		public boolean isLauncherUI(AccessibilityNodeInfo root) {
			AccessibilityNodeInfo node = CavanAccessibilityHelper.getChildRecursiveF(root, 0, -1, 0, -1);
			if (node == null) {
				return false;
			}

			try {
				if (CavanAccessibilityHelper.isNodeClassEquals(node, FrameLayout.class)) {
					return false;
				}

				AccessibilityNodeInfo child = CavanAccessibilityHelper.getChildRecursiveF(node, 0, 0, 0);
				if (child != null) {
					String desc = CavanAccessibilityHelper.getNodeDescription(child);

					child.recycle();

					if ("服务按钮".equals(desc)) {
						return false;
					}
				}
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				node.recycle();
			}

			return true;
		}

		@Override
		public void onEnter(AccessibilityNodeInfo root) {
			int hashCode = root.hashCode();
			if (hashCode != mHashCode) {
				mHashCode = hashCode;
				CavanAndroid.dLog("mHashCode = " + Integer.toHexString(hashCode));
				performPackageUpdated();
			}

			setHomePending(false);
			setUnfollowPending(false);
			setForceUnpackEnable(true);
		}

		@Override
		public void onWindowContentChanged(AccessibilityNodeInfo root, AccessibilityEvent event) {
			AccessibilityNodeInfo source = event.getSource();
			if (source != null) {
				if (CavanAccessibilityHelper.isNodeClassEquals(source, RelativeLayout.class) && isLauncherUI(root)) {
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
						setGotoHome(true);
						setPending(true);
						break;
					}
				}

				CavanAccessibilityHelper.recycleNodes(mChangedNodes);
				mChangedNodes.clear();
			}

			return true;
		}

		@Override
		protected boolean doSignin(AccessibilityNodeInfo root) {
			return clickOfficialAccounts(root);
		}

		@Override
		protected boolean doFollow(AccessibilityNodeInfo root) {
			return clickOfficialAccounts(root);
		}

		@Override
		protected boolean doActionBack(AccessibilityNodeInfo root) {
			return doActionBackBase(root, 0, 0, 0, 0, 0);
		}

		@Override
		protected boolean doActionHome(AccessibilityNodeInfo root) {
			AccessibilityNodeInfo input = findInputNode(root);
			if (input != null) {
				input.recycle();
			} else {
				doActionBack(root);
			}

			setHomePending(false);

			return true;
		}

		@Override
		public int getEventTypes(CavanAccessibilityPackage pkg) {
			return super.getEventTypes(pkg) | AccessibilityEvent.TYPE_WINDOW_CONTENT_CHANGED;
		}
	}

	public class ContactInfoWindow extends BaseWindow {

		public ContactInfoWindow(String name) {
			super(name);
		}

		public List<AccessibilityNodeInfo> getListViews(AccessibilityNodeInfo root) {
			return CavanAccessibilityHelper.getChildsByClassNameF(root, ListView.class.getCanonicalName(), 0);
		}

		public boolean enterOfficialAccounts(AccessibilityNodeInfo root) {
			AccessibilityNodeInfo node = CavanAccessibilityHelper.getChildRecursiveF(root, 0, -1, 2, 0);
			CavanAndroid.dLog("node = " + node);
			if (node != null) {
				String text = CavanAccessibilityHelper.getNodeText(node);

				try {
					if (CavanString.mach(text, "关注", "发消息", "进入公众号", "关注公众号")) {
						if (CavanAccessibilityHelper.performClick(node)) {
							return true;
						}

						return CavanAccessibilityHelper.performClickParent(node);
					}
				} catch (Exception e) {
					e.printStackTrace();
				} finally {
					node.recycle();
				}
			}

			List<AccessibilityNodeInfo> listViews = getListViews(root);
			if (listViews == null || listViews.isEmpty()) {
				return false;
			}

			try {
				for (AccessibilityNodeInfo listView : listViews) {
					for (int i = 0; i < 3; i++) {
						List<AccessibilityNodeInfo> nodes = CavanAccessibilityHelper.findNodesByTexts(listView, "关注", "发消息", "进入公众号", "关注公众号");
						if (nodes != null && nodes.size() > 0) {
							try {
								node = nodes.get(nodes.size() - 1);

								if (CavanAccessibilityHelper.performClick(node)) {
									return true;
								}

								return CavanAccessibilityHelper.performClickParent(node);
							} catch (Exception e) {
								e.printStackTrace();
							} finally {
								CavanAccessibilityHelper.recycleNodes(nodes);
							}
						}

						if (i > 0) {
							CavanAccessibilityHelper.performScrollDown(listView);
						}

						CavanJava.msleep(200);
					}
				}
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				CavanAccessibilityHelper.recycleNodes(listViews);
			}

			return false;
		}

		@Override
		protected boolean doSignin(AccessibilityNodeInfo root) {
			return enterOfficialAccounts(root);
		}

		@Override
		protected boolean doFollow(AccessibilityNodeInfo root) {
			return enterOfficialAccounts(root);
		}

		@Override
		protected boolean doUnfollow(AccessibilityNodeInfo root) {
			List<AccessibilityNodeInfo> listViews = getListViews(root);
			if (listViews != null && listViews.size() > 0) {
				try {
					for (AccessibilityNodeInfo listView : listViews) {
						for (int i = 0; i < 3; i++) {
							List<AccessibilityNodeInfo> nodes = CavanAccessibilityHelper.findNodesByTexts(listView, "取消关注", "不再关注");
							if (nodes != null && nodes.size() > 0) {
								try {
									AccessibilityNodeInfo node = nodes.get(nodes.size() - 1);

									if (CavanAccessibilityHelper.performClick(node)) {
										return true;
									}

									return CavanAccessibilityHelper.performClickParent(node);
								} catch (Exception e) {
									e.printStackTrace();
								} finally {
									CavanAccessibilityHelper.recycleNodes(nodes);
								}
							}

							if (i > 0) {
								CavanAccessibilityHelper.performScrollDown(listView);
							}

							CavanJava.msleep(200);
						}
					}
				} catch (Exception e) {
					e.printStackTrace();
				} finally {
					CavanAccessibilityHelper.recycleNodes(listViews);
				}
			}

			AccessibilityNodeInfo node = CavanAccessibilityHelper.getChildRecursiveF(root, 0, 2);
			if (node == null) {
				return false;
			}

			try {
				return CavanAccessibilityHelper.performClick(node);
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				node.recycle();
			}

			return false;
		}

		@Override
		protected boolean doActionBack(AccessibilityNodeInfo root) {
			return doActionBackBase(root, 0, 1, 0) || doActionBackBase(root, 0, 0, 0);
		}

		@Override
		protected boolean doActionHome(AccessibilityNodeInfo root) {
			return doActionBack(root);
		}

		@Override
		protected void onEnter(AccessibilityNodeInfo root) {
			if (isSigninPending()) {
				doSignin(root);
			} else if (isFollowPending()) {
				doFollow(root);
			} else if (isUnfollowPending()) {
				doUnfollow(root);
			}  else if (isHomePending()) {
				doActionHome(root);
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
		public void onEnter(AccessibilityNodeInfo root) {
			if (isForceUnpackEnabled()) {
				setPending(true);
			}
		}

		@Override
		public void onLeave(AccessibilityNodeInfo root) {
			setForceUnpackEnable(false);
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
		public void onEnter(AccessibilityNodeInfo root) {
			if (getCurrentPacket() == null) {
				setPending(false);
			}
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

		@Override
		protected boolean doActionBack(AccessibilityNodeInfo root) {
			AccessibilityNodeInfo node = findBackNode(root);
			if (node == null) {
				return false;
			}

			return CavanAccessibilityHelper.performClickAndRecycle(node);
		}
	}

	public class WebViewWindow extends BaseWindow {

		private boolean isTmpWebviewUi;

		public WebViewWindow(String name) {
			super(name);
			isTmpWebviewUi = name.indexOf("Tmp") > 0;
		}

		public AccessibilityNodeInfo findWebView(AccessibilityNodeInfo root) {
			ArrayList<AccessibilityNodeInfo> nodes = new ArrayList<>();

			try {
				AccessibilityNodeInfo parent = root.getChild(0);
				if (parent == null) {
					return null;
				}

				nodes.add(parent);

				for (int i = 0; i < parent.getChildCount(); i++) {
					AccessibilityNodeInfo child = parent.getChild(i);
					if (child == null) {
						return null;
					}

					nodes.add(child);

					if (!CavanAccessibilityHelper.isInstanceOf(child, FrameLayout.class)) {
						continue;
					}

					AccessibilityNodeInfo node = CavanAccessibilityHelper.getChildRecursiveF(child, 0, 0);
					if (node == null) {
						continue;
					}

					if (CavanAccessibilityHelper.isInstanceOf(node, WebView.class)) {
						return node;
					}

					nodes.add(node);
				}
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				CavanAccessibilityHelper.recycleNodes(nodes);
			}

			return null;
		}

		public boolean isInProgress(AccessibilityNodeInfo root) {
			if (isTmpWebviewUi) {
				return false;
			}

			AccessibilityNodeInfo node = CavanAccessibilityHelper.getChildRecursiveF(root, 0, 3);
			if (node == null) {
				return false;
			}

			try {
				return CavanAccessibilityHelper.isInstanceOf(node, ProgressBar.class);
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				node.recycle();
			}

			return false;
		}

		public String getTitle(AccessibilityNodeInfo root) {
			AccessibilityNodeInfo node = CavanAccessibilityHelper.getChildRecursiveF(root, 0, 3, 1);
			if (node == null) {
				return null;
			}

			try {
				return CavanAccessibilityHelper.getNodeText(node);
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				node.recycle();
			}

			return null;
		}

		@Override
		public boolean isWebviewUi() {
			return isCurrentPackage();
		}

		public boolean clickMenuButton(AccessibilityNodeInfo root) {
			AccessibilityNodeInfo node = CavanAccessibilityHelper.getChildRecursiveF(root, 0, -1);
			if (node == null) {
				return false;
			}

			try {
				if (CavanAccessibilityHelper.isDescriptionEquals(node, "更多")) {
					return CavanAccessibilityHelper.performClick(node);
				}
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				node.recycle();
			}

			return false;
		}

		@Override
		protected boolean doRefresh(AccessibilityNodeInfo root) {
			if (isInProgress(root)) {
				CavanAndroid.dLog("isInProgress");
				return true;
			}

			if (clickMenuButton(root)) {
				mMenuItem = "刷新";
				return true;
			}

			return false;
		}

		@Override
		protected boolean doCommandShare(AccessibilityNodeInfo root, boolean friends) {
			if (!clickMenuButton(root)) {
				return false;
			}

			if (friends) {
				mMenuItem = "发送给朋友";
			} else {
				mMenuItem = "分享到朋友圈";
			}

			return true;
		}

		@Override
		protected boolean doActionBack(AccessibilityNodeInfo root) {
			AccessibilityNodeInfo[] nodes = CavanAccessibilityHelper.getChildsRecursiveF(root, 0, 3, 0, 0);
			if (nodes == null) {
				return false;
			}

			try {
				if ("返回".equals(CavanAccessibilityHelper.getNodeDescription(nodes[3]))) {
					return CavanAccessibilityHelper.performClick(nodes[2]);
				}
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				CavanAccessibilityHelper.recycleNodes(nodes);
			}

			return false;
		}

		@Override
		protected boolean doActionHome(AccessibilityNodeInfo root) {
			return doActionBack(root);
		}

		@Override
		protected boolean doSignin(AccessibilityNodeInfo root) {
			setSigninPending(false);

			String title = getTitle(root);
			if (title == null) {
				return false;
			}

			AccessibilityNodeInfo node;

			if (title.contains("穿越火线")) {
				while (true) {
					node = CavanAccessibilityHelper.getChildRecursiveF(root, 0, 0, 0, 0, 0, 2, 1);
					if (node == null) {
						return false;
					}

					String desc = CavanAccessibilityHelper.getNodeDescription(node);
					CavanAndroid.dLog("desc = " + desc);
					if (desc.contains("登录")) {
						break;
					}

					node.recycle();

					node = CavanAccessibilityHelper.getChildRecursiveF(root, 0, 0, 0, 0, 0, 2, 3);
					if (node == null) {
						return false;
					}

					desc = CavanAccessibilityHelper.getNodeDescription(node);
					CavanAndroid.dLog("desc = " + desc);
					if (desc.contains("注销")) {
						mService.postCommand(200, 3, CMD_SIGNIN);
						break;
					}

					node.recycle();

					return false;
				}
			} else {
				return false;
			}

			try {
				CavanAccessibilityHelper.performClick(node);
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				node.recycle();
			}

			return false;
		}

		@Override
		protected void onEnter(AccessibilityNodeInfo root) {
			if (isSigninPending()) {
				mService.postCommand(1500, 5, CMD_SIGNIN);
			} else if (isHomePending()) {
				doActionHome(root);
			}
		}

		@Override
		protected boolean doWebCommand(AccessibilityNodeInfo root, String action) {
			AccessibilityNodeInfo web = findWebView(root);
			if (web == null) {
				return false;
			}

			try {
				switch (action) {
				case "vip":
					return doWebCommandVip(root, web);

				case "xfzd":
					return doWebCommandXfzd(root, web);
				}
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				web.recycle();
			}

			return false;
		}

		private boolean doWebCommandXfzd(AccessibilityNodeInfo root, AccessibilityNodeInfo web) {
			ArrayList<AccessibilityNodeInfo> nodes = new ArrayList<>();

			try {
				AccessibilityNodeInfo node = web.getChild(0);
				if (node == null) {
					return false;
				}

				nodes.add(node);

				String id = CavanAccessibilityHelper.getNodeViewId(node);
				CavanAndroid.dLog("id = " + id);

				if ("waiting".equals(id)) {
					return true;
				}

				while (node.getChildCount() > 0) {
					AccessibilityNodeInfo child = node.getChild(0);
					if (child == null) {
						break;
					}

					nodes.add(child);
					node = child;
				}

				return CavanAccessibilityHelper.performClick(node);
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				CavanAccessibilityHelper.recycleNodes(nodes);
			}

			return false;
		}

		private boolean doWebCommandVip(AccessibilityNodeInfo root, AccessibilityNodeInfo web) {
			AccessibilityNodeInfo node = CavanAccessibilityHelper.getChildRecursiveF(web, 0, 0, 0, 0, 0);
			if (node != null) {
				return CavanAccessibilityHelper.performClickAndRecycle(node);
			}

			return false;
		}
	}

	public class WebViewMenu extends MenuWindow {

		public WebViewMenu(String name) {
			super(name);
		}

		@Override
		public boolean isWebviewUi() {
			return isCurrentPackage();
		}

		@Override
		public boolean doClickMenuItem(AccessibilityNodeInfo root, String item) {
			AccessibilityNodeInfo node = CavanAccessibilityHelper.findNodeByText(root, item);
			if (node == null) {
				return false;
			}

			AccessibilityNodeInfo parent = node.getParent();

			try {
				if (parent != null) {
					return CavanAccessibilityHelper.performClick(parent);
				}
			} catch (Exception e) {
				if (parent != null) {
					parent.recycle();
				}

				node.recycle();
			}

			return false;
		}

		@Override
		protected boolean doRefresh(AccessibilityNodeInfo root) {
			return doClickMenuItem(root, "刷新");
		}

		@Override
		protected boolean doCommandShare(AccessibilityNodeInfo root, boolean friends) {
			if (friends) {
				return doClickMenuItem(root, "发送给朋友");
			} else {
				return doClickMenuItem(root, "分享到朋友圈");
			}
		}

		@Override
		protected boolean doUnfollow(AccessibilityNodeInfo root) {
			return doClickMenuItem(root, "不再关注");
		}

		@Override
		protected void onEnter(AccessibilityNodeInfo root) {
			if (isUnfollowPending()) {
				doUnfollow(root);
			} else {
				super.onEnter(root);
			}
		}
	}

	public class AppBrandMenuWindow extends WebViewMenu {

		public AppBrandMenuWindow(String name) {
			super(name);
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

		@Override
		public boolean isPopWindow() {
			return true;
		}
	}

	public class MobileInputWindow extends BaseWindow {

		public MobileInputWindow(String name) {
			super(name);
		}

		@Override
		protected boolean doLogin(AccessibilityNodeInfo root, String username, String password) {
			AccessibilityNodeInfo node = CavanAccessibilityHelper.getChildRecursiveF(root, 0, 2, 3);
			if (node == null) {
				return false;
			}

			try {
				if (CavanAccessibilityHelper.isButton(node)) {
					return CavanAccessibilityHelper.performClick(node);
				}
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				node.recycle();
			}

			return false;
		}
	};

	public class LoginPasswordWindow extends BaseWindow {

		public LoginPasswordWindow(String name) {
			super(name);
		}

		public boolean setInputText(AccessibilityNodeInfo root, String label, String text) {
			AccessibilityNodeInfo node = CavanAccessibilityHelper.findNodeByText(root, label);
			if (node == null) {
				return false;
			}

			AccessibilityNodeInfo parent = null;
			AccessibilityNodeInfo input = null;

			try {
				parent = node.getParent();
				if (parent == null) {
					return false;
				}

				input = CavanAccessibilityHelper.getChild(parent, 1);
				if (input == null) {
					return false;
				}

				if (CavanAccessibilityHelper.isEditText(input) && CavanAccessibilityHelper.setNodeText(mService, input, text) != null) {
					return true;
				}
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				if (input != null) {
					input.recycle();
				}

				if (parent != null) {
					parent.recycle();
				}

				node.recycle();
			}

			return false;
		}

		public boolean setPassword(AccessibilityNodeInfo root, String password) {
			return setInputText(root, "密码", password);
		}

		public boolean setPassword(AccessibilityNodeInfo root) {
			String username = getUserName(root);
			if (username == null) {
				return false;
			}

			String password = mService.getPassword(CavanAccessibilityMM.this, username);
			if (password == null) {
				return false;
			}

			return setPassword(root, password);
		}

		public boolean clickLoginButton(AccessibilityNodeInfo root) {
			AccessibilityNodeInfo node = CavanAccessibilityHelper.findNodeByText(root, "登录");
			if (node == null) {
				return false;
			}

			try {
				if (CavanAccessibilityHelper.isButton(node)) {
					return CavanAccessibilityHelper.performClick(node);
				}
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				node.recycle();
			}

			return false;
		}

		public String getUserName(AccessibilityNodeInfo root) {
			AccessibilityNodeInfo node = CavanAccessibilityHelper.getChildRecursiveF(root, 2, 0);
			if (node == null) {
				return null;
			}

			try {
				return CavanAccessibilityHelper.getNodeText(node);
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				node.recycle();
			}

			return null;
		}

		@Override
		protected void onEnter(AccessibilityNodeInfo root) {
			setPassword(root);
		}

		@Override
		protected boolean doLogin(AccessibilityNodeInfo root, String username, String password) {
			String usernameNow = getUserName(root);
			if (usernameNow == null) {
				return false;
			}

			if (username == null) {
				username = usernameNow;
			} else if (!username.equals(usernameNow)) {
				return false;
			}

			CavanAndroid.dLog("username = " + username);

			if (password == null) {
				password = mService.getPassword(CavanAccessibilityMM.this, username);
				if (password == null) {
					return false;
				}
			}

			if (!setPassword(root, password)) {
				return false;
			}

			return clickLoginButton(root);
		}
	}

	public class LoginWindow extends LoginPasswordWindow {

		public LoginWindow(String name) {
			super(name);
		}

		public boolean setUserName(AccessibilityNodeInfo root, String username) {
			if (username != null) {
				return setInputText(root, "帐号", username);
			}

			mService.showLoginDialog(CavanAccessibilityMM.this);

			return false;
		}

		@Override
		protected void onEnter(AccessibilityNodeInfo root) {
			mService.showLoginDialog(CavanAccessibilityMM.this);
		}

		@Override
		protected boolean doLogin(AccessibilityNodeInfo root, String username, String password) {
			if (!setUserName(root, username)) {
				return false;
			}

			if (!setPassword(root, password)) {
				return false;
			}

			return clickLoginButton(root);
		}
	}

	public class AppBrandAnswer {

		private AccessibilityNodeInfo mNode;
		private String mText;

		public AppBrandAnswer(AccessibilityNodeInfo node) {
			mNode = node;

			String text = CavanAccessibilityHelper.getChildDesction(node, -1);
			if (text != null) {
				int index = text.indexOf('.');
				if (index > 0) {
					mText = text.substring(index + 1).trim();
				}
			}
		}

		public String getText() {
			return mText;
		}

		public void recycle() {
			if (mNode != null) {
				mNode.recycle();
				mNode = null;
			}
		}

		public boolean isHashEquals(int hashCode) {
			return (mNode != null && mNode.hashCode() == hashCode);
		}

		public boolean click() {
			if (mNode != null) {
				return mService.doInputTap(mNode);
			}

			return false;
		}

		@Override
		public String toString() {
			if (mText != null) {
				return mText;
			}

			return "none";
		}
	}

	public class AppBrandAnswers {

		private AccessibilityNodeInfo mNode;
		private AppBrandAnswer[] mAnswers;

		public AppBrandAnswers(AccessibilityNodeInfo node) {
			mNode = node;

			AccessibilityNodeInfo[] childs = CavanAccessibilityHelper.getChilds(node);
			if (childs != null) {
				mAnswers = new AppBrandAnswer[childs.length];
				for (int i = 0; i < childs.length; i++) {
					mAnswers[i] = new AppBrandAnswer(childs[i]);
				}
			}
		}

		public void recycle() {
			if (mAnswers != null) {
				for (AppBrandAnswer answer : mAnswers) {
					answer.recycle();
				}

				mAnswers = null;
			}

			if (mNode != null) {
				mNode.recycle();
				mNode = null;
			}
		}

		public AppBrandAnswer getAnswer(AccessibilityNodeInfo node) {
			if (mAnswers == null) {
				return null;
			}

			for (AppBrandAnswer answer : mAnswers) {
				if (answer.isHashEquals(node.hashCode())) {
					return answer;
				}
			}

			return null;
		}

		public AppBrandAnswer getAnswer(String text) {
			for (AppBrandAnswer answer : mAnswers) {
				if (text.equals(answer.getText())) {
					return answer;
				}
			}

			return null;
		}

		public boolean isValid() {
			if (mAnswers == null || mAnswers.length < 2) {
				return false;
			}

			return true;
		}

		public void dump() {
			AppBrandAnswer[] answers = mAnswers;
			if (answers != null) {
				for (int i = 0; i < answers.length; i++) {
					CavanAndroid.dLog("answer" + i + ". " + answers[i].getText());
				}
			}
		}

		public StringBuilder toString(StringBuilder builder) {
			AppBrandAnswer[] answers = mAnswers;

			builder.append("[ ");

			if (answers != null) {
				for (int i = 0; i < answers.length; i++) {
					if (i > 0) {
						builder.append(", ");
					}

					builder.append(answers[i].getText());
				}
			}

			builder.append(" ]");

			return builder;
		}

		@Override
		public String toString() {
			return toString(new StringBuilder()).toString();
		}
	}

	public class AppBrandSubject {

		private AccessibilityNodeInfo mNode;
		private String mQuestion;
		private AppBrandAnswers mAnswers;
		private AppBrandAnswer mAnswer;
		private AppBrandAnswer mAnswerCurr;

		public AppBrandSubject(AccessibilityNodeInfo node) {
			mNode = node;

			String question = CavanAccessibilityHelper.getChildDesction(node, 0);
			if (question != null) {
				int index = question.indexOf('.');
				if (index > 0) {
					mQuestion = question.substring(index + 1).trim();
				}
			}

			AccessibilityNodeInfo answer = CavanAccessibilityHelper.getChild(node, 1);
			if (answer != null) {
				mAnswers = new AppBrandAnswers(answer);
			}
		}

		public String getQuestion() {
			return mQuestion;
		}

		public void recycle() {
			if (mAnswers != null) {
				mAnswers.recycle();
				mAnswers = null;
			}

			if (mNode != null) {
				mNode.recycle();
				mNode = null;
			}
		}

		public void setCurrentAnswer(AccessibilityNodeInfo source) {
			mAnswerCurr = mAnswers.getAnswer(source);
		}

		public AppBrandAnswer getCurrentAnswer() {
			return mAnswerCurr;
		}

		public void save() {
			AppBrandAnswer answer = getCurrentAnswer();
			if (answer != null && mQuestion != null) {
				CavanAndroid.dLog(mQuestion + " <= " + answer.getText());

				synchronized (mSubjects) {
					mSubjects.put(mQuestion, answer.getText());
				}

				mService.doSaveSubject(mQuestion, answer.getText());
			}
		}

		public void remove() {
			AppBrandAnswer answer = getCurrentAnswer();

			if (answer == mAnswer && mQuestion != null) {
				CavanAndroid.dLog("remove: " + mQuestion);

				synchronized (mSubjects) {
					mSubjects.remove(mQuestion);
				}

				mService.doRemoveSubject(mQuestion);
			}
		}

		public AppBrandAnswer getAnswer() {
			if (mQuestion == null) {
				return null;
			}

			String answer;

			synchronized (mSubjects) {
				answer = mSubjects.get(mQuestion);
				if (answer == null) {
					return null;
				}
			}

			CavanAndroid.dLog(mQuestion + " => " + answer);

			return mAnswers.getAnswer(answer);
		}

		public void setAnswer(AppBrandAnswer answer) {
			CavanAndroid.dLog("mAutoAnswer = " + mAutoAnswer);

			mAnswer = answer;

			if (mAutoAnswer) {
				answer.click();
			}
		}

		public boolean isValid() {
			return (mAnswers != null && mAnswers.isValid());
		}

		public void dump() {
			CavanAndroid.dLog("question. " + mQuestion);
			mAnswers.dump();
		}

		@Override
		public String toString() {
			StringBuilder builder = new StringBuilder();

			if (mQuestion != null) {
				builder.append(mQuestion);
			}

			builder.append('@');

			if (mAnswers != null) {
				mAnswers.toString(builder);
			}

			return builder.toString();
		}

		@Override
		public boolean equals(Object o) {
			if (o instanceof AppBrandSubject) {
				AppBrandSubject subject = (AppBrandSubject) o;
				String question = subject.getQuestion();
				if (question != null) {
					return question.equals(mQuestion);
				}
			}

			return false;
		}
	};

	public String getAppBrandTitle0(AccessibilityNodeInfo root) {
		AccessibilityNodeInfo node = CavanAccessibilityHelper.getChildRecursiveF(root, 0, 0, 0);
		if (node == null) {
			return null;
		}

		try {
			CharSequence text = node.getText();
			if (text != null) {
				return text.toString();
			}
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			node.recycle();
		}

		return null;
	}

	public String getAppBrandTitle1(AccessibilityNodeInfo root) {
		AccessibilityNodeInfo node = CavanAccessibilityHelper.getChildRecursiveF(root, 0, 1, 0);
		if (node == null) {
			return null;
		}

		try {
			CharSequence text = node.getText();
			if (text != null) {
				return text.toString();
			}
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			node.recycle();
		}

		return null;
	}

	public String getAppBrandTitle(AccessibilityNodeInfo root) {
		String title = getAppBrandTitle0(root);
		if (title != null) {
			return title;
		}

		return getAppBrandTitle1(root);
	}

	public boolean isAnswerCorrect(AccessibilityNodeInfo root) {
		AccessibilityNodeInfo node = CavanAccessibilityHelper.getChildRecursiveF(root, 0, 3, 0, 0, 1, 2);
		if (node == null) {
			return false;
		}

		try {
			String text = CavanAccessibilityHelper.getNodeDescription(node);
			CavanAndroid.dLog("text = " + text);

			if (text != null && text.contains("秒")) {
				return true;
			}
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			node.recycle();
		}

		return false;
	}

	public boolean setSubject(AccessibilityNodeInfo root, AppBrandSubject subject) {
		if (subject != null && subject.isValid()) {
			if (mSubject != null) {
				if (subject.equals(mSubject)) {
					return false;
				}

				mSubject.save();
				mSubject.recycle();
				mSubject = null;
			}

			AppBrandAnswer answer = subject.getAnswer();
			if (answer != null) {
				subject.setAnswer(answer);
			}

			mSubject = subject;

			return true;
		}

		if (mSubject != null) {
			if (isAnswerCorrect(root)) {
				mSubject.save();
			} else {
				mSubject.remove();
			}

			mSubject.recycle();
			mSubject = null;
		}

		if (subject != null) {
			subject.recycle();
		}

		return false;
	}

	public boolean isAnswerCompleted(AccessibilityNodeInfo root) {
		AccessibilityNodeInfo node = CavanAccessibilityHelper.getChildRecursiveF(root, 0, 3, 0, 0, 10, 0);
		if (node == null) {
			return false;
		}

		try {
			return "再次答题".equals(CavanAccessibilityHelper.getNodeDescription(node));
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			node.recycle();
		}

		return false;
	}

	public boolean doAutoAnswer(AccessibilityNodeInfo root) {
		String title = getAppBrandTitle(root);
		if (title == null) {
			return false;
		}

		CavanAndroid.dLog("title = " + title);

		if (title.equals("争分夺金")) {
			AccessibilityNodeInfo node = CavanAccessibilityHelper.getChildRecursiveF(root, 0, -1, 0, 0, -1);
			if (node != null) {
				AppBrandSubject subject = new AppBrandSubject(node);
				setSubject(root, subject);
				subject.dump();
				return false;
			}

			if (isAnswerCompleted(root)) {
				setSubject(root, null);
				return false;
			}

			return true;
		} else if (title.equals("答题")) {
			AccessibilityNodeInfo node = CavanAccessibilityHelper.getChildRecursiveF(root, 0, 3, 0);
			if (node != null) {
				try {
					int a = CavanJava.parseInt(CavanAccessibilityHelper.getChildDesction(node, 3));
					int b = CavanJava.parseInt(CavanAccessibilityHelper.getChildDesction(node, 5));
					String answer0 = CavanAccessibilityHelper.getChildDesction(node, 6);
					String op = CavanAccessibilityHelper.getChildDesction(node, 4);
					int answer1;

					CavanAndroid.dLog("a = " + a);
					CavanAndroid.dLog("b = " + b);
					CavanAndroid.dLog("op = " + op);
					CavanAndroid.dLog("answer0 = " + answer0);

					if (answer0.charAt(0) != '=') {
						return false;
					}

					if (op.charAt(0) == '-') {
						answer1 = a - b;
					} else if (op.charAt(0) == '+') {
						answer1 = a + b;
					} else {
						return false;
					}

					CavanAndroid.dLog("answer1 = " + answer1);

					AccessibilityNodeInfo child;

					if (answer1 == CavanJava.parseInt(answer0.substring(1))) {
						child = CavanAccessibilityHelper.getChild(node, 7);
					} else {
						child = CavanAccessibilityHelper.getChild(node, 9);
					}

					CavanAndroid.dLog("child = " + child);

					if (child != null) {
						mService.doInputTap(child);
						child.recycle();
					}
				} catch (Exception e) {
					e.printStackTrace();
				} finally {
					node.recycle();
				}
			}
		} else if (title.equals("包你拼-拼字/拼图")) {
			AccessibilityNodeInfo node0 = null;
			AccessibilityNodeInfo node1 = null;
			AccessibilityNodeInfo[] childs = null;

			try {
				node0 = CavanAccessibilityHelper.getChildRecursiveF(root, 0, 3, 0, 2, 0, 0, 0, 2, 0);
				if (node0 == null) {
					return false;
				}

				node1 = CavanAccessibilityHelper.getChildRecursiveF(root, 0, 3, 0, 4);
				if (node1 == null) {
					return false;
				}

				childs = CavanAccessibilityHelper.getChilds(node1);
				if (childs == null) {
					return false;
				}

				String text = CavanAccessibilityHelper.getNodeDescription(node0);
				for (int i = 0; i < text.length(); i++) {
					char c = text.charAt(i);

					for (AccessibilityNodeInfo child : childs) {
						if (CavanAccessibilityHelper.getNodeDescription(child).charAt(0) == c) {
							mService.doInputTap(child);
							break;
						}
					}

					CavanJava.msleep(100);
				}
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				if (childs != null) {
					CavanAccessibilityHelper.recycleNodes(childs);
				}

				if (node1 != null) {
					node1.recycle();
				}

				if (node0 != null) {
					node0.recycle();
				}
			}
		}

		return false;
	}

	public boolean doAutoAnswer() {
		AccessibilityNodeInfo root = getRootInActiveWindow();
		if (root == null) {
			return false;
		}

		try {
			return doAutoAnswer(root);
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			root.recycle();
		}

		return false;
	}

	public void postAutoAnswer(long delay, int times) {
		mAnswerTime = System.currentTimeMillis() + delay;
		mAnswerTimes = times;

		if (delay > 0) {
			postDelayed(mRunnableAutoAnswer, delay);
		} else {
			post(mRunnableAutoAnswer);
		}
	}

	public void cancelAutoAnswer(AccessibilityNodeInfo root) {
		cancel(mRunnableAutoAnswer);
		setSubject(root, null);
		mAutoAnswer = false;
	}

	public class AppBrandWindow extends BaseWindow {

		public AppBrandWindow(String name) {
			super(name);
		}

		public boolean performClickMoreButton(AccessibilityNodeInfo root) {
			AccessibilityNodeInfo[] nodes = CavanAccessibilityHelper.getChildsRecursiveF(root, 1, 0, 0);
			if (nodes == null) {
				return false;
			}

			try {
				if (CavanAccessibilityHelper.isInstanceOf(nodes[2], ImageButton.class)) {
					return CavanAccessibilityHelper.performClick(nodes[1]);
				}
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				CavanAccessibilityHelper.recycleNodes(nodes);
			}

			return false;
		}

		@Override
		protected void onViewClicked(AccessibilityNodeInfo root, AccessibilityEvent event) {
			if (mSubject != null) {
				AccessibilityNodeInfo source = event.getSource();
				if (source != null) {
					try {
						mSubject.setCurrentAnswer(source);
					} catch (Exception e) {
						e.printStackTrace();
					} finally {
						source.recycle();
					}
				}

				postAutoAnswer(1200, 0);
			}
		}

		@Override
		protected void onKeyDown(AccessibilityNodeInfo root, int keyCode) {
			if (keyCode == KeyEvent.KEYCODE_VOLUME_DOWN) {
				postAutoAnswer(0, 0);
			} else if (keyCode == KeyEvent.KEYCODE_VOLUME_UP) {
				mAutoAnswer = true;
			}
		}

		@Override
		protected void onEnter(AccessibilityNodeInfo root) {
			cancelAutoAnswer(root);
		}

		@Override
		protected void onLeave(AccessibilityNodeInfo root) {
			cancelAutoAnswer(root);
		}

		@Override
		public int getEventTypes(CavanAccessibilityPackage pkg) {
			return super.getEventTypes(pkg) | AccessibilityEvent.TYPE_VIEW_CLICKED;
		}

		@Override
		protected boolean doCommandShare(AccessibilityNodeInfo root, boolean friends) {
			if (performClickMoreButton(root)) {
				mMenuItem = "转发";
				return true;
			}

			return false;
		}

		@Override
		protected boolean doActionBack(AccessibilityNodeInfo root) {
			AccessibilityNodeInfo node = CavanAccessibilityHelper.getChildRecursiveF(root, 1, 1);
			if (node == null) {
				return false;
			}

			return CavanAccessibilityHelper.performClickAndRecycle(node);
		}
	};

	public class DialogWindow extends BaseWindow {

		public DialogWindow(String name) {
			super(name);
		}

		@Override
		public boolean isPopWindow() {
			return true;
		}

		@Override
		protected boolean doUnfollow(AccessibilityNodeInfo root) {
			for (int  i = 0; i < 3; i++) {
				AccessibilityNodeInfo node = CavanAccessibilityHelper.findNodeByText(root, "不再关注");
				if (node != null) {
					return CavanAccessibilityHelper.performClickAndRecycle(node);
				}

				CavanAndroid.dLog("retry = " + i);
				CavanJava.msleep(100);
			}

			return mBrandServiceIndexWindow.doUnfollow(root);
		}

		@Override
		protected void onEnter(AccessibilityNodeInfo root) {
			if (isUnfollowPending()) {
				doUnfollow(root);
			} else if (isSharePending()) {
				doCommandShare(root, false);
			}
		}

		@Override
		protected boolean doWebCommand(AccessibilityNodeInfo root, String action) {
			if (action.equals("vip")) {
				return CavanAccessibilityHelper.performChildClick(root, 1);
			}

			return false;
		}

		@Override
		protected boolean doCommandShare(AccessibilityNodeInfo root, boolean friends) {
			setSharePending(false);

			AccessibilityNodeInfo node = CavanAccessibilityHelper.getChildByIndex(root, 4);
			if (node == null) {
				return false;
			}

			try {
				if ("发送".equals(CavanAccessibilityHelper.getNodeText(node))) {
					CavanAccessibilityHelper.performClick(node);
				}
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				node.recycle();
			}

			return false;
		}
	}

	public class BrandServiceIndexWindow extends BaseWindow {

		public BrandServiceIndexWindow(String name) {
			super(name);
		}

		@Override
		protected boolean doUnfollow(AccessibilityNodeInfo root) {
			AccessibilityNodeInfo node = CavanAccessibilityHelper.getChildRecursiveF(root, 0, 4, 0);
			if (node == null) {
				return false;
			}

			if (CavanAccessibilityHelper.performLongClickAndRecycle(node)) {
				mMenuItem = "取消关注";
				return true;
			}

			return false;
		}
	}

	public CavanAccessibilityMM(CavanAccessibilityService service) {
		super(service, CavanPackageName.MM);
	}

	public boolean isWebViewUi() {
		BaseWindow win = (BaseWindow) mWindow;
		return (win != null && win.isWebviewUi() && isCurrentPackage());
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
		} else if (name.startsWith("com.tencent.mm.plugin.appbrand.ui.AppBrandUI")) {
			win = new AppBrandWindow(name);
		} else if (name.startsWith("com.tencent.mm.plugin.webview.ui.tools.")) {
			win = new WebViewWindow(name);
		} else {
			return null;
		}

		addWindow(win);

		return win;
	}

	public class ExtDeviceWXLoginWindow extends BaseWindow {

		public ExtDeviceWXLoginWindow(String name) {
			super(name);
		}

		@Override
		protected void onEnter(AccessibilityNodeInfo root) {
			List<AccessibilityNodeInfo> nodes = CavanAccessibilityHelper.findNodesByText(root, "登录");
			if (nodes != null) {
				try {
					for (AccessibilityNodeInfo node : nodes) {
						if (CavanAccessibilityHelper.isButton(node)) {
							CavanAccessibilityHelper.performClick(node);
						}
					}
				} catch (Exception e) {
					e.printStackTrace();
				} finally {
					CavanAccessibilityHelper.recycleNodes(nodes);
				}
			}
		}
	}

	public class SelectConversationWindow extends BaseWindow {

		public SelectConversationWindow(String name) {
			super(name);
		}

		@Override
		protected boolean doCommandShare(AccessibilityNodeInfo root, boolean friends) {
			AccessibilityNodeInfo node = CavanAccessibilityHelper.findNodeByText(root, "两口");
			if (node == null) {
				return false;
			}

			return CavanAccessibilityHelper.performClickParentAndRecycle(node);
		}

		@Override
		protected void onEnter(AccessibilityNodeInfo root) {
			CavanAndroid.dLog("isSharePending = " + isSharePending());
			if (isSharePending()) {
				CavanAndroid.pLog();
				doCommandShare(root, false);
			}
		}
	}

	public class SnsUploadWindow extends BaseWindow {

		public SnsUploadWindow(String name) {
			super(name);
		}

		@Override
		protected boolean doCommandShare(AccessibilityNodeInfo root, boolean friends) {
			AccessibilityNodeInfo node = CavanAccessibilityHelper.getChildRecursiveF(root, 0, 0, 3);
			if (node == null) {
				return false;
			}

			try {
				if ("谁可以看".equals(CavanAccessibilityHelper.getChildText(node, 0))) {
					String type = CavanAccessibilityHelper.getChildText(node, 1);
					if (type == null) {
						return false;
					}

					if (type.equals("公开")) {
						return CavanAccessibilityHelper.performClick(node);
					}

					setSharePending(false);

					AccessibilityNodeInfo publish = CavanAccessibilityHelper.getChild(root, 3);
					if (publish != null) {
						CavanAccessibilityHelper.performClickAndRecycle(publish);
					}

					return false;
				}
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				node.recycle();
			}

			return false;
		}

		@Override
		protected void onEnter(AccessibilityNodeInfo root) {
			if (isSharePending()) {
				doCommandShare(root, false);
			}
		}
	}

	public class SnsLabelWindow extends BaseWindow {

		public SnsLabelWindow(String name) {
			super(name);
		}

		@Override
		protected boolean doCommandShare(AccessibilityNodeInfo root, boolean friends) {
			AccessibilityNodeInfo node = CavanAccessibilityHelper.findNodeByText(root, "私密");
			if (node == null) {
				return false;
			}

			if (!CavanAccessibilityHelper.performClickParentAndRecycle(node)) {
				return false;
			}

			node = CavanAccessibilityHelper.findNodeByText(root, "完成");
			if (node == null) {
				return false;
			}

			return CavanAccessibilityHelper.performClickAndRecycle(node);
		}

		@Override
		protected void onEnter(AccessibilityNodeInfo root) {
			if (isSharePending()) {
				doCommandShare(root, false);
			}
		}
	}

	public boolean doActionBackBase(AccessibilityNodeInfo root, Object... indexs) {
		AccessibilityNodeInfo[] nodes = CavanAccessibilityHelper.getChildsRecursive(root, indexs);
		if (nodes == null) {
			return false;
		}

		try {
			if ("返回".equals(CavanAccessibilityHelper.getNodeDescription(nodes[nodes.length - 1]))) {
				return CavanAccessibilityHelper.performClick(nodes[nodes.length - 2]);
			}
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			CavanAccessibilityHelper.recycleNodes(nodes);
		}

		return false;
	}

	public boolean isWxViewPager(AccessibilityNodeInfo node) {
		return CavanAccessibilityHelper.isInstanceOf(node, "com.tencent.mm.ui.mogic.WxViewPager");
	}

	@Override
	public void initWindows() {
		mBrandServiceIndexWindow = new BrandServiceIndexWindow("com.tencent.mm.plugin.brandservice.ui.BrandServiceIndexUI");
		addWindow(mBrandServiceIndexWindow);

		addWindow(new LauncherWindow("com.tencent.mm.ui.LauncherUI"));
		addWindow(new ChattingWindow("com.tencent.mm.ui.chatting.ChattingUI"));
		addWindow(new ChattingWindow("com.tencent.mm.ui.conversation.BizConversationUI"));
		addWindow(new ReceiveWindow("com.tencent.mm.plugin.luckymoney.ui.LuckyMoneyReceiveUI"));
		addWindow(new ReceiveWindow("com.tencent.mm.plugin.luckymoney.ui.LuckyMoneyBusiReceiveUI"));
		addWindow(new ReceiveWindow("com.tencent.mm.plugin.luckymoney.ui.LuckyMoneyNotHookReceiveUI"));
		addWindow(new DetailWindow("com.tencent.mm.plugin.luckymoney.ui.LuckyMoneyDetailUI"));
		addWindow(new DetailWindow("com.tencent.mm.plugin.luckymoney.ui.LuckyMoneyBusiDetailUI"));

		addWindow(new WebViewWindow("com.tencent.mm.plugin.webview.ui.tools.WebViewUI"));
		addWindow(new WebViewWindow("com.tencent.mm.plugin.webview.ui.tools.WebviewMpUI"));
		addWindow(new WebViewWindow("com.tencent.mm.plugin.webview.ui.tools.game.GameWebViewUI"));

		addWindow(new WebViewMenu("android.support.design.widget.c"));
		addWindow(new ChattingMenu("android.widget.FrameLayout"));
		addWindow(new MobileInputWindow("com.tencent.mm.ui.account.MobileInputUI"));
		addWindow(new MobileInputWindow("com.tencent.mm.plugin.account.ui.MobileInputUI"));

		addWindow(new LoginWindow("com.tencent.mm.ui.account.LoginUI"));
		addWindow(new LoginWindow("com.tencent.mm.ui.account.SimpleLoginUI"));
		addWindow(new LoginPasswordWindow("com.tencent.mm.ui.account.LoginPasswordUI"));

		addWindow(new LoginWindow("com.tencent.mm.plugin.account.ui.LoginUI"));
		addWindow(new LoginWindow("com.tencent.mm.plugin.account.ui.SimpleLoginUI"));
		addWindow(new LoginPasswordWindow("com.tencent.mm.plugin.account.ui.LoginPasswordUI"));
		addWindow(new ExtDeviceWXLoginWindow("com.tencent.mm.plugin.webwx.ui.ExtDeviceWXLoginUI"));

		addWindow(new ContactInfoWindow("com.tencent.mm.plugin.profile.ui.ContactInfoUI"));
		addWindow(new DialogWindow("com.tencent.mm.ui.base.i"));
		addWindow(new DialogWindow("com.tencent.mm.ui.widget.a.c"));
		addWindow(new AppBrandMenuWindow("com.tencent.mm.ui.widget.a.b"));

		addWindow(new SelectConversationWindow("com.tencent.mm.ui.transmit.SelectConversationUI"));
		addWindow(new SnsUploadWindow("com.tencent.mm.plugin.sns.ui.SnsUploadUI"));
		addWindow(new SnsLabelWindow("com.tencent.mm.plugin.sns.ui.SnsLabelUI"));
	}

	@Override
	public void onPollStarted() {
		mFinishNodes.clear();
	}

	@Override
	public void onNotificationStateChanged(AccessibilityNodeInfo root, Notification notification) {
		CavanNotificationMM packet = new CavanNotificationMM(notification);
		if (packet.isRedPacket()) {
			addPacket(packet);
		}
	}

	@Override
	protected void onCreate() {
		super.onCreate();
		instance = this;

		synchronized (mSubjects) {
			mService.doLoadSubjects(mSubjects);
		}
	}

	@Override
	protected void onDestroy() {
		instance = null;
		super.onDestroy();
	}
}
