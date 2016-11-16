package com.cavan.cavanmain;

import java.util.List;

import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanPackageName;

public class CavanAccessibilitySogou extends CavanAccessibilityBase {

	public CavanAccessibilitySogou(CavanAccessibilityService service) {
		super(service);
	}

	@Override
	public String getPackageName() {
		return CavanPackageName.SOGOU_IME;
	}

	@Override
	public void onWindowStateChanged(AccessibilityEvent event) {
		mService.startCheckAutoOpenApp();

		AccessibilityNodeInfo source = event.getSource();
		if (source == null) {
			return;
		}

		if ("com.sogou.ocrplugin.OCRResultActivity".equals(mClassName)) {
			List<AccessibilityNodeInfo> nodes = source.findAccessibilityNodeInfosByViewId("com.sohu.inputmethod.sogou:id/result_view");
			if (nodes != null && nodes.size() > 0) {
				CharSequence sequence = nodes.get(0).getText();
				if (sequence != null) {
					mService.doCheckContent(sequence.toString());
				}
			}
		}
	}
}
