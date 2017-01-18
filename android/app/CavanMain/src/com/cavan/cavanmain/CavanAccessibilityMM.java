package com.cavan.cavanmain;

import java.util.ArrayList;
import java.util.List;

import android.os.Build;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanAccessibility;
import com.cavan.android.CavanPackageName;

public class CavanAccessibilityMM extends CavanAccessibilityBase {

	private static final int POLL_DELAY = 500;

	private List<String> mRedPackets = new ArrayList<String>();
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
	public void addRedPacket(Object packet) {
		int delay = MainActivity.getAutoUnpackMM(mService);
		if (delay < 0) {
			return;
		}

		if (mRedPackets.contains(packet)) {
			return;
		}

		mFinishNodes.clear();
		mRedPackets.add((String) packet);
		setLockEnable(POLL_DELAY, false);
	}

	@Override
	public int getRedPacketCount() {
		return mRedPackets.size();
	}

	@Override
	public void clearRedPackets() {
		mRedPackets.clear();
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
				if (parent.getChildCount() == 3 && "领取红包".equals(CavanAccessibility.getChildText(parent, 1))) {
					mFinishNodes.add(hash);
					setLockEnable(POLL_DELAY, false);
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

			mRedPackets.clear();
			startNextActivity();
			return 0;

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
}
