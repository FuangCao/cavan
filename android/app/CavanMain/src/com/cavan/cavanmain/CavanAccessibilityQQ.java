package com.cavan.cavanmain;

import android.content.Intent;
import android.graphics.Rect;
import android.os.Build;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;

public class CavanAccessibilityQQ extends CavanAccessibilityBase {

	private String mMessageBoxText;

	public CavanAccessibilityQQ(CavanAccessibilityService service) {
		super(service);
	}

	public boolean isMessageBoxNode(AccessibilityNodeInfo node) {
		if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
			String id = node.getViewIdResourceName();
			return "com.tencent.mobileqq:id/msgbox".equals(id);
		}

		if (node.isMultiLine()) {
			return false;
		}

		if (CavanAccessibilityService.isTextView(node)) {
			Rect bounds = new Rect();

			node.getBoundsInScreen(bounds);
			if (bounds.width() < mService.getDisplayWidth()) {
				return false;
			}

			node.getBoundsInParent(bounds);
			if (bounds.top != 0) {
				return false;
			}

			return true;
		}

		return false;
	}

	@Override
	public String getPackageName() {
		return CavanPackageName.QQ;
	}

	@Override
	public void onWindowStateChanged(AccessibilityEvent event) {
		if ("com.tencent.mobileqq.activity.aio.photo.AIOGalleryActivity".equals(mClassName)) {
			mService.setAutoOpenAppEnable(false);
		}
	}

	@Override
	public void onWindowContentChanged(AccessibilityEvent event) {
		AccessibilityNodeInfo source = event.getSource();
		if (source != null && isMessageBoxNode(source)) {
			CharSequence sequence = source.getText();
			if (sequence == null) {
				return;
			}

			String text = sequence.toString();
			if (text.equals(mMessageBoxText)) {
				return;
			}

			mMessageBoxText = text;

			if (text.contains("[QQ红包]")) {
				source.performAction(AccessibilityNodeInfo.ACTION_CLICK);
			}

			if (!CavanAndroid.inKeyguardRestrictedInputMode(mService)) {
				Intent intent = new Intent(MainActivity.ACTION_CONTENT_RECEIVED);
				intent.putExtra("package", source.getPackageName());
				intent.putExtra("desc", "QQ消息盒子");
				intent.putExtra("content", text);
				intent.putExtra("hasPrefix", true);
				mService.sendBroadcast(intent);
			}
		}
	}

	@Override
	public void onViewClicked(AccessibilityEvent event) {
		AccessibilityNodeInfo source = event.getSource();
		if (source == null) {
			return;
		}

		if (CavanAccessibilityService.isTextView(source)) {
			AccessibilityNodeInfo parent = source.getParent();
			if (parent == null) {
				return;
			}

			String id = parent.getViewIdResourceName();
			if (id == null) {
				return;
			}

			if (id.equals("com.tencent.mobileqq:id/chat_item_content_layout") ||
					id.equals("com.tencent.mobileqq:id/name")) {
				postMessageNode(source);
			}
		}
	}
}
