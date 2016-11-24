package com.cavan.cavanmain;

import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanPackageName;

public class CavanAccessibilityMM extends CavanAccessibilityBase {

	public CavanAccessibilityMM(CavanAccessibilityService service) {
		super(service);
	}

	@Override
	public String getPackageName() {
		return CavanPackageName.MM;
	}

	@Override
	protected void onViewClicked(AccessibilityEvent event) {
		AccessibilityNodeInfo source = event.getSource();

		if (source != null && source.isMultiLine() && CavanAccessibilityService.isTextView(source)) {
			postMessageNode(source);
		}
	}
}
