package com.cavan.weixinredpacket;

import java.util.HashMap;
import java.util.Set;

import android.accessibilityservice.AccessibilityService;
import android.accessibilityservice.AccessibilityServiceInfo;
import android.app.Notification;
import android.os.Build;
import android.os.Parcelable;
import android.view.KeyEvent;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;
import com.cavan.java.CavanString;

public class CavanAccessibilityService extends AccessibilityService {

	private long mWindowStartTime;
	private String mClassName = CavanString.EMPTY_STRING;
	private String mPackageName = CavanString.EMPTY_STRING;

	private CavanAccessibilityMM mAccessibilityMM = new CavanAccessibilityMM(this);
	private HashMap<String, CavanAccessibilityBase<?>> mAccessibilityMap = new HashMap<String, CavanAccessibilityBase<?>>();

	public CavanAccessibilityService() {
		super();

		mAccessibilityMap.put(CavanPackageName.MM, mAccessibilityMM);
	}

	public CavanAccessibilityMM getAccessibilityMM() {
		return mAccessibilityMM;
	}

	public long getWindowTimeConsume() {
		return System.currentTimeMillis() - mWindowStartTime;
	}

	public String getRootPackageName() {
		AccessibilityNodeInfo info = getRootInActiveWindow();
		if (info == null) {
			return null;
		}

		CharSequence pkg = info.getPackageName();
		info.recycle();

		if (pkg == null) {
			return null;
		}

		return pkg.toString();
	}

	public boolean isRootActivity(String pkgName) {
		return pkgName.equals(getRootPackageName());
	}

	public String getActivityClassName() {
		return mClassName;
	}

	public String getActivityPackageName() {
		return mPackageName;
	}

	public boolean startIdleActivity() {
		return performGlobalAction(AccessibilityService.GLOBAL_ACTION_HOME);
	}

	@Override
	public void onAccessibilityEvent(AccessibilityEvent event) {
		CharSequence sequence = event.getPackageName();
		if (sequence == null) {
			return;
		}

		mPackageName = sequence.toString();

		CavanAccessibilityBase<?> accessibility = mAccessibilityMap.get(mPackageName);
		if (accessibility == null) {
			return;
		}

		switch (event.getEventType()) {
		case AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED:
			mWindowStartTime = System.currentTimeMillis();

			sequence = event.getClassName();
			if (sequence != null) {
				String className = sequence.toString();

				if (!className.startsWith("android.widget.")) {
					mClassName = className;
				}
			}

			CavanAndroid.dLog("package = " + mPackageName);
			CavanAndroid.dLog("class = " + mClassName);

			accessibility.performWindowStateChanged(event, mPackageName, mClassName);
			break;

		case AccessibilityEvent.TYPE_WINDOW_CONTENT_CHANGED:
			accessibility.onWindowContentChanged(event);
			break;

		case AccessibilityEvent.TYPE_VIEW_TEXT_CHANGED:
			accessibility.onViewTextChanged(event);
			break;

		case AccessibilityEvent.TYPE_NOTIFICATION_STATE_CHANGED:
			Parcelable data = event.getParcelableData();
			if (data instanceof Notification) {
				accessibility.onNotificationStateChanged((Notification) data);
			}
			break;
		}
	}

	@Override
	protected boolean onKeyEvent(KeyEvent event) {
		AccessibilityNodeInfo root = getRootInActiveWindow();
		if (root == null) {
			return false;
		}

		CharSequence sequence = root.getPackageName();
		if (sequence == null) {
			return false;
		}

		CavanAndroid.dLog("package = " + sequence);

		CavanAccessibilityBase<?> accessibility = mAccessibilityMap.get(sequence.toString());
		if (accessibility == null) {
			return false;
		}

		return accessibility.onKeyEvent(event);
	}

	@Override
	protected void onServiceConnected() {
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
			AccessibilityServiceInfo info = getServiceInfo();

			Set<String> keys = mAccessibilityMap.keySet();
			info.packageNames = new String[keys.size()];
			keys.toArray(info.packageNames);

			info.flags |= AccessibilityServiceInfo.DEFAULT |
					AccessibilityServiceInfo.FLAG_REPORT_VIEW_IDS |
					AccessibilityServiceInfo.FLAG_REQUEST_FILTER_KEY_EVENTS |
					AccessibilityServiceInfo.FLAG_RETRIEVE_INTERACTIVE_WINDOWS;

			info.eventTypes = AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED | AccessibilityEvent.TYPE_NOTIFICATION_STATE_CHANGED;

			setServiceInfo(info);

			CavanAndroid.dLog("info = " + getServiceInfo());
		}

		super.onServiceConnected();
	}

	@Override
	public void onInterrupt() {
		CavanAndroid.pLog();
	}
}
