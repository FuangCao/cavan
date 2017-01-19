package com.cavan.cavanmain;

import java.util.ArrayList;
import java.util.List;

import android.os.Build;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanAccessibility;
import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;

public class CavanAccessibilityMM extends CavanAccessibilityBase<String> {

	private static final int POLL_DELAY = 500;
	private static final int POLL_DELAY_UNPACK = 2000;

	private List<Integer> mFinishNodes = new ArrayList<Integer>();

	public CavanAccessibilityMM(CavanAccessibilityService service) {
		super(service);
	}

	private boolean isMessageItemNode(AccessibilityNodeInfo node) {
		if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
			String id = node.getViewIdResourceName();
			return "com.tencent.mm:id/ib".equals(id);
		}

		return node.isMultiLine() && CavanAccessibility.isTextView(node);
	}

	@Override
	public String getPackageName() {
		return CavanPackageName.MM;
	}

	@Override
	public void addPacket(String packet) {
		int delay = MainActivity.getAutoUnpackMM(mService);
		if (delay < 0) {
			return;
		}

		if (mPackets.contains(packet)) {
			return;
		}

		mFinishNodes.clear();
		mPackets.add(packet);
		setLockEnable(POLL_DELAY, false);
	}

	@Override
	protected void onViewClicked(AccessibilityEvent event) {
		AccessibilityNodeInfo source = event.getSource();
		if (source != null && isMessageItemNode(source)) {
			postMessageNode(source);
		}
	}

	private boolean doUnpack(AccessibilityNodeInfo root) {
		AccessibilityNodeInfo backNode = CavanAccessibility.findNodeByViewId(root, "com.tencent.mm:id/bed");
		if (backNode == null) {
			return false;
		}

		AccessibilityNodeInfo button = CavanAccessibility.findNodeByViewId(root, "com.tencent.mm:id/be_");
		if (button != null) {
			setLockEnable(POLL_DELAY, false);
			CavanAccessibility.performClickAndRecycle(button);
		} else {
			setLockEnable(POLL_DELAY, false);
			CavanAccessibility.performClick(backNode);
		}

		backNode.recycle();

		return true;
	}

	private boolean isValidMessage(AccessibilityNodeInfo node) {
		if (node.getChildCount() != 3) {
			return false;
		}

		String message = CavanAccessibility.getChildText(node, 0);
		CavanAndroid.dLog("message = " + message);

		if (message == null || message.contains("测") || message.contains("挂")) {
			return false;
		}

		String action = CavanAccessibility.getChildText(node, 1);
		CavanAndroid.dLog("action = " + action);

		return "领取红包".equals(action);
	}

	private boolean doFindAndUnpack(AccessibilityNodeInfo root) {
		boolean success = false;

		List<AccessibilityNodeInfo> nodes = CavanAccessibility.findNodesByText(root, "微信红包");
		for (int i = nodes.size() - 1; i >= 0; i--) {
			AccessibilityNodeInfo node = nodes.get(i);
			int hash = node.hashCode();

			if (mFinishNodes.contains(hash)) {
				continue;
			}

			AccessibilityNodeInfo parent = node.getParent();
			if (parent != null) {
				if (isValidMessage(parent)) {
					mFinishNodes.add(hash);
					setLockEnable(POLL_DELAY_UNPACK, false);
					CavanAccessibility.performClickAndRecycle(parent);
					success = true;
					break;
				}

				parent.recycle();
			}
		}

		CavanAccessibility.recycleNodes(nodes);

		return success;
	}

	@Override
	protected long onPollEventFire(AccessibilityNodeInfo root) {
		switch (mClassName) {
		case "com.tencent.mm.ui.LauncherUI":
			if (doFindAndUnpack(root)) {
				break;
			}

			mPackets.clear();
			break;

		case "com.tencent.mm.plugin.luckymoney.ui.LuckyMoneyReceiveUI":
			doUnpack(root);
			break;

		case "com.tencent.mm.plugin.luckymoney.ui.LuckyMoneyDetailUI":
			AccessibilityNodeInfo backNode = CavanAccessibility.findNodeByViewId(root, "com.tencent.mm:id/gr");
			if (backNode != null) {
				setLockEnable(POLL_DELAY, false);
				CavanAccessibility.performClickAndRecycle(backNode);
			}
			break;

		case "com.tencent.mm.ui.base.p":
			break;

		default:
			return 0;
		}

		return POLL_DELAY;
	}

	@Override
	public void onWindowStateChanged(AccessibilityEvent event) {
		switch (mClassName) {
		case "com.tencent.mm.ui.LauncherUI":
		case "com.tencent.mm.plugin.luckymoney.ui.LuckyMoneyReceiveUI":
		case "com.tencent.mm.plugin.luckymoney.ui.LuckyMoneyDetailUI":
			if (getPacketCount() > 0) {
				setLockEnable(POLL_DELAY, true);
			}
			break;

		default:
			if (getPacketCount() > 0) {
				setLockEnable(POLL_DELAY, false);
			}
		}
	}
}
