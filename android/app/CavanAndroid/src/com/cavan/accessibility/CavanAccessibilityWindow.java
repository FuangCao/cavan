package com.cavan.accessibility;

import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;


public abstract class CavanAccessibilityWindow {

	private CavanAccessibilityPackage<?> mPackage;

	public CavanAccessibilityWindow(CavanAccessibilityPackage<?> pkg) {
		mPackage = pkg;
	}

	public CavanAccessibilityPackage<?> getPackage() {
		return mPackage;
	}

	public abstract int run(AccessibilityNodeInfo root);

	public void enter() {}
	public void leave() {}
	public void onWindowContentChanged(AccessibilityEvent event) {}
	public void onViewClicked(AccessibilityEvent event) {}
	public void onViewTextChanged(AccessibilityEvent event) {}
}
