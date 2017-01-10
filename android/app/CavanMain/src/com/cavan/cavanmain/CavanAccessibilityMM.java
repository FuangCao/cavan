package com.cavan.cavanmain;

import android.os.Build;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanAccessibility;
import com.cavan.android.CavanPackageName;

public class CavanAccessibilityMM extends CavanAccessibilityBase {

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
	public int getRedPacketCount() {
		return 0;
	}

	@Override
	protected void onViewClicked(AccessibilityEvent event) {
		AccessibilityNodeInfo source = event.getSource();
		if (source != null && isMessageItemNode(source)) {
			postMessageNode(source);
		}
	}
}