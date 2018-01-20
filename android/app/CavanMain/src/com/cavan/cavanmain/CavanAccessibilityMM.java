package com.cavan.cavanmain;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import android.view.View;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.cavan.android.CavanAccessibilityHelper;
import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;

public class CavanAccessibilityMM extends CavanAccessibilityBase<String> {

	private static final int POLL_DELAY = 500;
	private static final int POLL_DELAY_UNPACK = 2000;

	private static CavanAccessibilityMM sInstance;

	public static CavanAccessibilityMM getInstance() {
		return sInstance;
	}

	private int mHashCode;
	private long mUnpackTime;
	private boolean mIsWebViewUi;
	private boolean mIsLauncherUi;
	private boolean mUnpackPending;
	private String mRedPacketViewId;
	private String mMessageListViewId;
	private List<Integer> mFinishNodes = new ArrayList<Integer>();

	public CavanAccessibilityMM(CavanAccessibilityService service) {
		super(service);
		sInstance = this;
	}

	public boolean isWebViewUi() {
		return mIsWebViewUi;
	}

	private boolean isMessageItemNode(AccessibilityNodeInfo node) {
		if (node.isFocusable() || node.isCheckable() || node.isScrollable() || node.isPassword()) {
			return false;
		}

		if (!node.isLongClickable()) {
			return false;
		}

		if (CavanAccessibilityHelper.isTextView(node)) {
			return node.isMultiLine();
		}

		if (View.class.getName().equals(node.getClassName())) {
			return (node.getChildCount() == 0);
		}

		return false;
	}

	private boolean updateUnpackTime() {
		int delay = CavanMessageActivity.getAutoUnpackMM(mService);
		if (delay < 0) {
			return false;
		}

		if (delay > 0) {
			long time = System.currentTimeMillis();
			if (mUnpackTime < time) {
				mUnpackTime = time + delay * 1000;
			}
		}

		return true;
	}

	@Override
	public String getPackageName() {
		return CavanPackageName.MM;
	}

	@Override
	public boolean addPacket(String packet) {
		if (updateUnpackTime() && super.addPacket(packet)) {
			mFinishNodes.clear();
			return true;
		}

		return false;
	}

	@Override
	protected void onViewClicked(AccessibilityEvent event) {
		if (mIsLauncherUi) {
			AccessibilityNodeInfo source = event.getSource();
			if (source != null && isMessageItemNode(source)) {
				postClickEventMessage(event);
			}
		}
	}

	private AccessibilityNodeInfo findReceiveUiBackNode(AccessibilityNodeInfo root) {
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

	private AccessibilityNodeInfo findReceiveUiUnpckNode(AccessibilityNodeInfo root) {
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

	private AccessibilityNodeInfo findDetailUiBackNode(AccessibilityNodeInfo root) {
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

	private long doUnpack(AccessibilityNodeInfo root) {
		long time = System.currentTimeMillis();
		if (mUnpackTime > time) {
			FloatMessageService service = FloatMessageService.getInstance();
			if (service != null) {
				service.setCountDownTime(mUnpackTime);
			}

			return mUnpackTime - time;
		}

		AccessibilityNodeInfo backNode = findReceiveUiBackNode(root);
		if (backNode == null) {
			return POLL_DELAY;
		}

		setLockEnable(POLL_DELAY, false);

		AccessibilityNodeInfo button = findReceiveUiUnpckNode(root);
		if (button != null) {
			CavanAccessibilityHelper.performClickAndRecycle(button);
		} else if (getPacketCount() > 0) {
			CavanAccessibilityHelper.performClick(backNode);
		} else {
			setForceUnpackEnable(false);
		}

		backNode.recycle();

		return POLL_DELAY;
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

	private boolean doFindAndUnpack(AccessibilityNodeInfo root) {
		AccessibilityNodeInfo listView = findMessageListViewNode(root);
		if (listView == null) {
			return false;
		}

		List<AccessibilityNodeInfo> nodes = findRedPacketNodes(listView);
		boolean success = false;

		for (int i = nodes.size() - 1; i >= 0; i--) {
			AccessibilityNodeInfo node = nodes.get(i);
			int hash = node.hashCode();

			if (mFinishNodes.contains(hash)) {
				continue;
			}

			if (isValidMessage(node)) {
				mFinishNodes.add(hash);
				setLockEnable(POLL_DELAY_UNPACK, false);
				CavanAccessibilityHelper.performClick(node);
				success = true;
				break;
			}
		}

		CavanAccessibilityHelper.recycleNodes(nodes);
		listView.recycle();

		return success;
	}

	private AccessibilityNodeInfo findMessageListViewNode(AccessibilityNodeInfo root) {
		if (mMessageListViewId != null) {
			AccessibilityNodeInfo node = CavanAccessibilityHelper.findNodeByViewId(root, mMessageListViewId);
			if (node == null || ListView.class.getName().equals(node.getClassName())) {
				return node;
			}
		}

		List<AccessibilityNodeInfo> childs = new ArrayList<AccessibilityNodeInfo>();

		try {
			AccessibilityNodeInfo child = root;

			while (child.getChildCount() > 0) {
				child = child.getChild(0);
				if (child == null) {
					break;
				}

				childs.add(child);

				if (LinearLayout.class.getName().equals(child.getClassName())) {
					AccessibilityNodeInfo node = CavanAccessibilityHelper.findChildByClassName(child, ListView.class.getName());
					if (node != null) {
						mMessageListViewId = node.getViewIdResourceName();
						CavanAndroid.dLog("mMessageListViewId = " + mMessageListViewId);
						return node;
					}
				}
			}
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			CavanAccessibilityHelper.recycleNodes(childs);
		}

		return null;
	}

	private boolean isRedPacketNode(AccessibilityNodeInfo node) {
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
		} finally {
			CavanAccessibilityHelper.recycleNodes(childs);
		}

		return true;
	}

	private List<AccessibilityNodeInfo> findRedPacketNodes(AccessibilityNodeInfo root) {
		if (mRedPacketViewId != null) {
			return root.findAccessibilityNodeInfosByViewId(mRedPacketViewId);
		}

		List<AccessibilityNodeInfo> nodes = new ArrayList<AccessibilityNodeInfo>();

		for (AccessibilityNodeInfo node : CavanAccessibilityHelper.findNodesByText(root, "微信红包")) {
			AccessibilityNodeInfo parent = node.getParent();
			if (parent != null) {
				if (isRedPacketNode(parent)) {
					mRedPacketViewId = parent.getViewIdResourceName();
					CavanAndroid.dLog("mRedPacketViewId = " + mRedPacketViewId);
					nodes.add(parent);
				} else {
					parent.recycle();
				}
			}

			node.recycle();
		}

		return nodes;
	}

	private AccessibilityNodeInfo findRedPacketNodeLast(AccessibilityNodeInfo root) {
		List<AccessibilityNodeInfo> nodes = findRedPacketNodes(root);
		if (nodes == null) {
			return null;
		}

		Iterator<AccessibilityNodeInfo> iterator = nodes.iterator();

		if (iterator.hasNext()) {
			AccessibilityNodeInfo node = iterator.next();

			while (iterator.hasNext()) {
				node.recycle();
				node = iterator.next();
			}

			return node;
		}

		return null;
	}

	@Override
	protected long onPollEventFire(AccessibilityNodeInfo root) {
		switch (mClassName) {
		case "com.tencent.mm.ui.LauncherUI":
		case "com.tencent.mm.ui.chatting.ChattingUI":
		case "com.tencent.mm.ui.conversation.BizConversationUI":
			if (doFindAndUnpack(root)) {
				break;
			}

			mPackets.clear();
			break;

		case "com.tencent.mm.plugin.luckymoney.ui.LuckyMoneyReceiveUI":
		case "com.tencent.mm.plugin.luckymoney.ui.LuckyMoneyBusiReceiveUI":
			return doUnpack(root);

		case "com.tencent.mm.plugin.luckymoney.ui.LuckyMoneyDetailUI":
		case "com.tencent.mm.plugin.luckymoney.ui.LuckyMoneyBusiDetailUI":
			if (mUnpackPending || getPacketCount() > 0) {
				AccessibilityNodeInfo backNode = findDetailUiBackNode(root);
				if (backNode != null) {
					mUnpackPending = false;
					setLockEnable(POLL_DELAY, false);
					CavanAccessibilityHelper.performClickAndRecycle(backNode);
				}
			} else {
				setForceUnpackEnable(false);
			}
			break;

		case "com.tencent.mm.ui.base.p":
			break;

		default:
			if (mClassName.startsWith("com.tencent.mm.plugin.luckymoney.ui.En_")) {
				return doUnpack(root);
			}
			return 0;
		}

		return POLL_DELAY;
	}

	@Override
	public void onWindowStateChanged(AccessibilityEvent event) {
		mIsLauncherUi = false;
		mIsWebViewUi = false;

		switch (mClassName) {
		case "com.tencent.mm.plugin.luckymoney.ui.LuckyMoneyBusiReceiveUI":
		case "com.tencent.mm.plugin.luckymoney.ui.LuckyMoneyReceiveUI":
			if (setForceUnpackEnable(true)) {
				break;
			}
		case "com.tencent.mm.plugin.luckymoney.ui.LuckyMoneyBusiDetailUI":
		case "com.tencent.mm.plugin.luckymoney.ui.LuckyMoneyDetailUI":
			if (getPacketCount() > 0) {
				setLockEnable(POLL_DELAY, true);
			}
			break;

		case "com.tencent.mm.ui.LauncherUI":
			AccessibilityNodeInfo root = getRootInActiveWindow();
			if (root != null) {
				int code = root.hashCode();
				if (code != mHashCode) {
					mHashCode = code;
					CavanAndroid.dLog("mHashCode = " + Integer.toHexString(code));

					mRedPacketViewId = null;
					mMessageListViewId = null;
				}

				root.recycle();
			}
		case "com.tencent.mm.ui.chatting.ChattingUI":
		case "com.tencent.mm.ui.conversation.BizConversationUI":
			mIsLauncherUi = true;
			setForceUnpackEnable(false);

			if (getPacketCount() > 0) {
				setLockEnable(POLL_DELAY, true);
			}
			break;

		case "com.tencent.mm.plugin.webview.ui.tools.WebViewUI":
			mIsWebViewUi = true;
			break;

		default:
			if (mClassName.startsWith("com.tencent.mm.plugin.luckymoney.ui.En_")) {
				if (setForceUnpackEnable(true)) {
					break;
				}

				if (getPacketCount() > 0) {
					setLockEnable(POLL_DELAY, true);
				}
			} else if (mClassName.startsWith("com.tencent.mm.ui.chatting.En_")) {
				mIsLauncherUi = true;
				setForceUnpackEnable(false);

				if (getPacketCount() > 0) {
					setLockEnable(POLL_DELAY, true);
				}
			} else if (getPacketCount() > 0) {
				setLockEnable(POLL_DELAY, false);
			}
		}
	}

	@Override
	protected boolean onWindowContentStable(int times) {
		if (!mIsLauncherUi) {
			return true;
		}

		AccessibilityNodeInfo root = getRootInActiveWindow();
		if (root == null) {
			return false;
		}

		AccessibilityNodeInfo listView = null;
		AccessibilityNodeInfo parent = null;
		AccessibilityNodeInfo node = null;

		try {
			listView = findMessageListViewNode(root);
			if (listView == null) {
				return false;
			}

			node = findRedPacketNodeLast(listView);
			if (node == null) {
				return false;
			}

			parent = node.getParent();
			if (parent == null) {
				return false;
			}

			boolean changed = mContentChangedHashCodes.contains(parent.hashCode());
			if (changed && isValidMessage(node) && updateUnpackTime()) {
				setLockEnable(POLL_DELAY_UNPACK, false);
				CavanAccessibilityHelper.performClick(node);
				mUnpackPending = true;
			}
		} catch (Exception e) {
			return false;
		} finally {
			if (parent != null) {
				parent.recycle();
			}

			if (node != null) {
				node.recycle();
			}

			if (listView != null) {
				listView.recycle();
			}

			root.recycle();
		}

		return true;
	}

	@Override
	public boolean commitText(AccessibilityNodeInfo root, CavanInputMethod ime) {
		if (mIsLauncherUi) {
			List<AccessibilityNodeInfo> nodes = CavanAccessibilityHelper.findNodesByText(root, "发送");
			int count = 0;

			for (AccessibilityNodeInfo node : nodes) {
				if (Button.class.getName().equals(node.getClassName()) && CavanAccessibilityHelper.performClick(node)) {
					count++;
				}
			}

			CavanAccessibilityHelper.recycleNodes(nodes);

			return count > 0;
		}

		return false;
	}

	@Override
	protected boolean onWindowContentChanged(AccessibilityEvent event, AccessibilityNodeInfo source) {
		return RelativeLayout.class.getName().equals(source.getClassName());
	}
}
