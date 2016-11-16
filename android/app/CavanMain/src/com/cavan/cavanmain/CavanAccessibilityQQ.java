package com.cavan.cavanmain;

import android.content.Intent;
import android.graphics.Rect;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;
import android.widget.TextView;

import com.cavan.android.CavanPackageName;
import com.cavan.java.CavanString;
import com.cavan.java.RedPacketFinder;

public class CavanAccessibilityQQ extends CavanAccessibilityBase {

	private static final String CLASS_NAME_TEXTVIEW = TextView.class.getName();

	private String mMessageBoxText;

	public CavanAccessibilityQQ(CavanAccessibilityService service) {
		super(service);
	}

	public boolean isMessageBoxNode(AccessibilityNodeInfo node) {
		if (node == null || node.isMultiLine()) {
			return false;
		}

		if (CLASS_NAME_TEXTVIEW.equals(node.getClassName())) {
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
			mService.startCheckAutoOpenApp();
		}
	}

	@Override
	public void onWindowContentChanged(AccessibilityEvent event) {
		AccessibilityNodeInfo source = event.getSource();
		if (isMessageBoxNode(source)) {
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

			Intent intent = new Intent(MainActivity.ACTION_CONTENT_RECEIVED);
			intent.putExtra("package", source.getPackageName());
			intent.putExtra("desc", "QQ消息盒子");
			intent.putExtra("content", text);
			intent.putExtra("hasPrefix", true);
			mService.sendBroadcast(intent);
		}
	}

	@Override
	public void onViewClicked(AccessibilityEvent event) {
		AccessibilityNodeInfo source = event.getSource();
		if (source == null) {
			return;
		}

		if (CLASS_NAME_TEXTVIEW.equals(source.getClassName().toString())) {
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
				String text = CavanString.fromCharSequence(source.getText());

				if (text.length() > 0 && RedPacketFinder.containsUrl(text) == false) {
					FloatEditorDialog dialog = FloatEditorDialog.getInstance(mService, text, true);
					dialog.show(6000);
				}

				Intent intent = new Intent(MainActivity.ACTION_CONTENT_RECEIVED);
				intent.putExtra("package", source.getPackageName());
				intent.putExtra("desc", "用户点击");
				intent.putExtra("content", text);
				mService.sendBroadcast(intent);
			}
		}
	}
}
