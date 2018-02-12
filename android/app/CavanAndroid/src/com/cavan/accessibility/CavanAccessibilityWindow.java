package com.cavan.accessibility;

import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;


public class CavanAccessibilityWindow {

	protected String mBackViewId;
	protected String mName;

	public CavanAccessibilityWindow(String name) {
		mName = name;
	}

	public String getName() {
		return mName;
	}

	public void setBackViewId(String id) {
		mBackViewId = id;
	}

	public String getBackViewId() {
		return mBackViewId;
	}

	public boolean poll(CavanRedPacket packet, AccessibilityNodeInfo root, int times) {
		return false;
	}

	public boolean performActionBack(AccessibilityNodeInfo root) {
		String vid = mBackViewId;
		if (vid == null) {
			return false;
		}

		return (CavanAccessibilityHelper.performClickByViewIds(root, vid) > 0);
	}

	public boolean performActionBack(CavanAccessibilityPackage pkg) {
		return pkg.getService().performActionBack();
	}

	public boolean performActionBack(AccessibilityNodeInfo root, CavanAccessibilityPackage pkg) {
		if (performActionBack(root)) {
			return true;
		}

		if (pkg == null) {
			return false;
		}

		return performActionBack(pkg);
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