package com.cavan.cavanmain;

import java.util.List;

import com.cavan.android.CavanPackageName;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.DialogInterface.OnClickListener;
import android.content.DialogInterface.OnDismissListener;
import android.os.Message;
import android.view.Gravity;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;
import android.widget.CheckBox;
import android.widget.EditText;

public class CavanAccessibilitySogou extends CavanAccessibilityBase {

	private static final int MSG_CHECK_CONTENT = 1;
	private static final int MSG_CHECK_AUTO_OPEN_APP = 2;

	private String mLastContent;
	private Dialog mCheckContentDialog;

	public CavanAccessibilitySogou(CavanAccessibilityService service) {
		super(service);
	}

	public void startCheckAutoOpenApp() {
		sendEmptyMessage(MSG_CHECK_AUTO_OPEN_APP);
	}

	@Override
	public void handleMessage(Message msg) {
		switch (msg.what) {
		case MSG_CHECK_CONTENT:
			if (mCheckContentDialog != null && mCheckContentDialog.isShowing()) {
				break;
			}

			String content = (String) msg.obj;

			if (msg.arg1 == 0 && mLastContent != null && mLastContent.equals(content)) {
				break;
			}

			mLastContent = content;

			AlertDialog.Builder builder = new AlertDialog.Builder(mService, R.style.DialogStyle);

			final View view = View.inflate(mService, R.layout.red_packet_check, null);
			final EditText editText = (EditText) view.findViewById(R.id.editTextContent);
			editText.setText(content);

			final CheckBox checkBox = (CheckBox) view.findViewById(R.id.checkBoxAsCode);

			builder.setView(view);
			builder.setNegativeButton(android.R.string.cancel, null);
			builder.setPositiveButton(R.string.text_send, new OnClickListener() {

				@Override
				public void onClick(DialogInterface dialog, int which) {
					String text = editText.getText().toString();

					MainActivity.setAutoOpenAppEnable(true);

					if (checkBox != null && checkBox.isChecked()) {
						if (text != null) {
							for (String line : text.split("\n")) {
								String code = RedPacketCode.filtration(line);

								if (code.length() > 0) {
									Intent intent = new Intent(MainActivity.ACTION_CODE_RECEIVED);
									intent.putExtra("type", "图片识别");
									intent.putExtra("code", code);
									intent.putExtra("shared", false);
									mService.sendBroadcast(intent);
								}
							}
						}
					} else {
						Intent intent = new Intent(MainActivity.ACTION_CONTENT_RECEIVED);
						intent.putExtra("desc", "图片识别");
						intent.putExtra("priority", 1);
						intent.putExtra("content", text);
						mService.sendBroadcast(intent);
					}
				}
			});

			builder.setOnDismissListener(new OnDismissListener() {

				@Override
				public void onDismiss(DialogInterface dialog) {
					mCheckContentDialog = null;
				}
			});

			mCheckContentDialog = builder.create();
			Window win = mCheckContentDialog.getWindow();

			win.setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
			if (msg.arg1 > 0) {
				win.setGravity(Gravity.CENTER_HORIZONTAL | Gravity.TOP);
				checkBox.setChecked(true);
				mCheckContentDialog.setCanceledOnTouchOutside(true);
			} else {
				mCheckContentDialog.setCancelable(false);
			}

			mCheckContentDialog.show();
			break;

		case MSG_CHECK_AUTO_OPEN_APP:
			removeMessages(MSG_CHECK_AUTO_OPEN_APP);

			if (getWindowTimeConsume() > 30000) {
				MainActivity.setAutoOpenAppEnable(true);
			} else if (mClassName.startsWith("com.sogou.ocrplugin") || mClassName.contains("gallery") ||
					mClassName.equals("com.tencent.mobileqq.activity.aio.photo.AIOGalleryActivity") ||
					mClassName.equals("android.app.Dialog")) {
				MainActivity.setAutoOpenAppEnable(false);
				sendEmptyMessageDelayed(MSG_CHECK_AUTO_OPEN_APP, 2000);
			} else {
				MainActivity.setAutoOpenAppEnable(true);
			}
			break;
		}
	}

	@Override
	public String getPackageName() {
		return CavanPackageName.SOGOU_IME;
	}

	@Override
	public void onWindowStateChanged(AccessibilityEvent event) {
		startCheckAutoOpenApp();

		AccessibilityNodeInfo source = event.getSource();
		if (source == null) {
			return;
		}

		if ("com.sogou.ocrplugin.OCRResultActivity".equals(mClassName)) {
			List<AccessibilityNodeInfo> nodes = source.findAccessibilityNodeInfosByViewId("com.sohu.inputmethod.sogou:id/result_view");
			if (nodes != null && nodes.size() > 0) {
				CharSequence sequence = nodes.get(0).getText();
				if (sequence != null) {
					Message message = obtainMessage(MSG_CHECK_CONTENT, sequence.toString());
					sendMessageDelayed(message, 500);
				}
			}
		} else {
			mLastContent = null;
		}
	}
}
