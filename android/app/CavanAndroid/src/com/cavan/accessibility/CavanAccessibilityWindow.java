package com.cavan.accessibility;

import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;


public abstract class CavanAccessibilityWindow {

	protected CavanAccessibilityPackage<?> mPackage;
	protected String mName;

	public CavanAccessibilityWindow(CavanAccessibilityPackage<?> pkg, String name) {
		mPackage = pkg;
		mName = name;
	}

	public String getName() {
		return mName;
	}

	public CavanAccessibilityPackage<?> getPackage() {
		return mPackage;
	}

	public boolean poll(AccessibilityNodeInfo root, int times) {
		return true;
	}

	public void onEnter() {}
	public void onLeave() {}
	public void onPackageUpdated() {}
	public void onWindowContentChanged(AccessibilityEvent event) {}
	public void onViewClicked(AccessibilityEvent event) {}
	public void onViewTextChanged(AccessibilityEvent event) {}

	public boolean onPollFailed(int times) {
		return (times < 5);
	}

	@Override
	public String toString() {
		return mName;
	}
}