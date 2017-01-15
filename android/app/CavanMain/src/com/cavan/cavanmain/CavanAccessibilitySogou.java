package com.cavan.cavanmain;

import java.util.List;

import android.view.KeyEvent;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanAccessibility;
import com.cavan.android.CavanPackageName;
import com.cavan.java.CavanString;

public class CavanAccessibilitySogou extends CavanAccessibilityBase {

	private String mLastClassName = CavanString.EMPTY_STRING;

	public CavanAccessibilitySogou(CavanAccessibilityService service) {
		super(service);
	}

	@Override
	public String getPackageName() {
		return CavanPackageName.SOGOU_IME;
	}

	@Override
	public int getRedPacketCount() {
		return 0;
	}

	@Override
	public void clearRedPackets() {
	}

	@Override
	public void onWindowStateChanged(AccessibilityEvent event) {
		mService.setAutoOpenAppEnable(false);

		if (mLastClassName.equals(mClassName)) {
			return;
		}

		mLastClassName = mClassName;

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

			CavanAccessibility.recycleNodes(nodes);
		}
	}

	@Override
	protected boolean onKeyEvent(KeyEvent event) {
		if (event.getKeyCode() == KeyEvent.KEYCODE_VOLUME_UP) {
			if (event.getAction() == KeyEvent.ACTION_DOWN) {
				mService.setAutoOpenAppEnable(true);
			}

			return true;
		}

		return false;
	}
}
