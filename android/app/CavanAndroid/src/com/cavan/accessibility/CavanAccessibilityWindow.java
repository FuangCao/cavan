package com.cavan.accessibility;

import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;


public class CavanAccessibilityWindow {

	protected String mName;

	public CavanAccessibilityWindow(String name) {
		mName = name;
	}

	public String getName() {
		return mName;
	}

	public boolean poll(CavanRedPacket packet, AccessibilityNodeInfo root, int times) {
		return false;
	}

	public boolean back(CavanAccessibilityPackage pkg, AccessibilityNodeInfo root) {
		return pkg.getService().performActionBack();
	}

	public void onEnter() {}
	public void onLeave() {}
	public void onPackageUpdated() {}
	public void onProgress(String name) {}
	public void onAndroidWidget(String name) {}
	public void onWindowContentChanged(AccessibilityEvent event) {}
	public void onViewClicked(AccessibilityEvent event) {}
	public void onViewTextChanged(AccessibilityEvent event) {}

	public boolean onPollFailed(CavanRedPacket packet, int times) {
		return (times < CavanAccessibilityPackage.FAIL_TIMES);
	}

	public boolean isMainActivity() {
		return false;
	}

	@Override
	public String toString() {
		return mName;
	}
}