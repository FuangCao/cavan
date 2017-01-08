package com.cavan.cavanmain;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import android.accessibilityservice.AccessibilityService;
import android.accessibilityservice.AccessibilityServiceInfo;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.DialogInterface.OnDismissListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.graphics.Point;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.TextView;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;
import com.cavan.java.CavanString;

public class CavanAccessibilityService extends AccessibilityService {

	public static final String CLASS_NAME_TEXTVIEW = TextView.class.getName();

	private static final int MSG_CHECK_CONTENT = 1;
	private static final int MSG_CHECK_AUTO_OPEN_APP = 2;

	private static final String[] PACKAGE_NAMES = {
		CavanPackageName.QQ,
		CavanPackageName.MM,
		CavanPackageName.ALIPAY,
		CavanPackageName.SOGOU_IME,
		CavanPackageName.SOGOU_OCR,
	};

	private Dialog mCheckContentDialog;

	private long mWindowStartTime;
	private String mClassName = CavanString.EMPTY_STRING;
	private String mPackageName = CavanString.EMPTY_STRING;

	private CavanAccessibilityQQ mAccessibilityQQ = new CavanAccessibilityQQ(this);
	private CavanAccessibilityMM mAccessibilityMM = new CavanAccessibilityMM(this);
	private CavanAccessibilitySogou mAccessibilitySogou = new CavanAccessibilitySogou(this);
	private CavanAccessibilityAlipay mAccessibilityAlipay = new CavanAccessibilityAlipay(this);
	private HashMap<String, CavanAccessibilityBase> mAccessibilityMap = new HashMap<String, CavanAccessibilityBase>();

	private Point mDisplaySize = new Point();

	private IFloatMessageService mService;
	private ServiceConnection mConnection = new ServiceConnection() {

		@Override
		public void onServiceDisconnected(ComponentName name) {
			mService = null;
			mAccessibilityAlipay.removeCodeAll();
		}

		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			mService = IFloatMessageService.Stub.asInterface(service);

			try {
				List<String> codes = mService.getCodes();
				if (codes != null && codes.size() > 0) {
					for (String code : codes) {
						RedPacketCode node = RedPacketCode.getInstence(code);
						if (node != null) {
							mAccessibilityAlipay.addCode(node);
						}
					}

					mAccessibilityAlipay.startAlipayActivity();
				}
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}
	};

	private BroadcastReceiver mReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();

			CavanAndroid.dLog("action = " + action);

			switch (action) {
			case MainActivity.ACTION_CODE_TEST:
				if (intent.getStringExtra("code") != null) {
					CavanAndroid.showToast(getApplicationContext(), R.string.text_test_sucess);
				}
				break;

			case MainActivity.ACTION_CODE_ADD:
				RedPacketCode node = RedPacketCode.getInstence(intent);
				if (node != null) {
					mAccessibilityAlipay.addCode(node);
					mAccessibilityAlipay.startAlipayActivity();
				}
				break;

			case MainActivity.ACTION_CODE_REMOVE:
				node = RedPacketCode.getInstence(intent);
				if (node != null) {
					mAccessibilityAlipay.removeCode(node);
				}
				break;

			case Intent.ACTION_CLOSE_SYSTEM_DIALOGS:
				mAccessibilityAlipay.setAutoOpenAlipayEnable(false);
				CavanAndroid.dLog("reason = " + intent.getStringExtra("reason"));
				break;

			case MainActivity.ACTION_UNPACK_MM:
				break;

			case MainActivity.ACTION_UNPACK_QQ:
				String chat = intent.getStringExtra("chat");
				if (chat != null) {
					mAccessibilityQQ.addRedPacket(chat);
				}
				break;
			}
		}
	};

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case MSG_CHECK_CONTENT:
				if (mCheckContentDialog != null && mCheckContentDialog.isShowing()) {
					break;
				}

				String content = (String) msg.obj;
				AlertDialog.Builder builder = new AlertDialog.Builder(CavanAccessibilityService.this, R.style.DialogStyle);

				final View view = View.inflate(CavanAccessibilityService.this, R.layout.red_packet_check, null);
				final EditText editText = (EditText) view.findViewById(R.id.editTextContent);
				editText.setText(content);

				final CheckBox checkBox = (CheckBox) view.findViewById(R.id.checkBoxAsCode);

				builder.setView(view);
				builder.setNegativeButton(android.R.string.cancel, null);
				builder.setPositiveButton(R.string.text_send, new OnClickListener() {

					@Override
					public void onClick(DialogInterface dialog, int which) {
						String text = editText.getText().toString();

						setAutoOpenAppEnable(true);

						if (checkBox != null && checkBox.isChecked()) {
							if (text != null) {
								for (String line : text.split("\n")) {
									String code = RedPacketCode.filtration(line);

									if (code.length() > 0) {
										Intent intent = new Intent(MainActivity.ACTION_CODE_RECEIVED);
										intent.putExtra("type", "图片识别");
										intent.putExtra("code", code);
										intent.putExtra("shared", false);
										sendBroadcast(intent);
									}
								}
							}
						} else {
							Intent intent = new Intent(MainActivity.ACTION_CONTENT_RECEIVED);
							intent.putExtra("desc", "图片识别");
							intent.putExtra("priority", 1);
							intent.putExtra("content", text);
							sendBroadcast(intent);
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

				if (needDisableAutoOpenApp()) {
					MainActivity.setAutoOpenAppEnable(false);
					sendEmptyMessageDelayed(MSG_CHECK_AUTO_OPEN_APP, 2000);
				} else {
					MainActivity.setAutoOpenAppEnable(true);
				}
				break;
			}
		}
	};

	public static boolean isTextView(AccessibilityNodeInfo node) {
		return CLASS_NAME_TEXTVIEW.equals(node.getClassName().toString());
	}

	public CavanAccessibilityService() {
		super();

		mAccessibilityMap.put(CavanPackageName.QQ, mAccessibilityQQ);
		mAccessibilityMap.put(CavanPackageName.MM, mAccessibilityMM);
		mAccessibilityMap.put(CavanPackageName.ALIPAY, mAccessibilityAlipay);
		mAccessibilityMap.put(CavanPackageName.SOGOU_IME, mAccessibilitySogou);
		mAccessibilityMap.put(CavanPackageName.SOGOU_OCR, mAccessibilitySogou);
	}

	public void setAutoOpenAppEnable(boolean enable) {
		if (enable) {
			mHandler.removeMessages(MSG_CHECK_AUTO_OPEN_APP);
			MainActivity.setAutoOpenAppEnable(true);
			mAccessibilityAlipay.setAutoOpenAlipayEnable(true);
		} else {
			mHandler.sendEmptyMessage(MSG_CHECK_AUTO_OPEN_APP);
		}
	}

	public long getWindowTimeConsume() {
		return System.currentTimeMillis() - mWindowStartTime;
	}

	public void doCheckContent(String content) {
		Message message = mHandler.obtainMessage(MSG_CHECK_CONTENT, content);
		mHandler.sendMessageDelayed(message, 500);
	}

	public boolean needDisableAutoOpenApp() {
		AccessibilityNodeInfo root = getRootInActiveWindow();
		if (root == null) {
			return false;
		}

		CharSequence sequence = root.getPackageName();
		if (sequence == null) {
			return false;
		}

		String packageName = sequence.toString();

		CavanAndroid.dLog("package = " + packageName);
		CavanAndroid.dLog("class = " + mClassName);

		if (mClassName.startsWith("com.sogou.ocrplugin")) {
			return packageName.contains("sogou") || packageName.contains("cavanmain");
		} else if (packageName.equals(CavanPackageName.QQ)) {
			return mClassName.equals("android.app.Dialog") || mClassName.equals("com.tencent.mobileqq.activity.aio.photo.AIOGalleryActivity");
		} else if (packageName.equals(CavanPackageName.GALLERY3D)) {
			return true;
		}

		return false;
	}

	public Point getDisplaySize() {
		return mDisplaySize;
	}

	public int getDisplayWidth() {
		return mDisplaySize.x;
	}

	public int getDisplayHeight() {
		return mDisplaySize.y;
	}

	public String getRootPackageName() {
		AccessibilityNodeInfo info = getRootInActiveWindow();
		if (info == null) {
			return null;
		}

		return info.getPackageName().toString();
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

	public static List<AccessibilityNodeInfo> findAccessibilityNodeInfosByTexts(AccessibilityNodeInfo root, String... texts) {
		List<AccessibilityNodeInfo> infos = new ArrayList<AccessibilityNodeInfo>();
		for (String text : texts) {
			List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByText(text);
			if (nodes == null) {
				continue;
			}

			for (AccessibilityNodeInfo node : nodes) {
				infos.add(node);
			}
		}

		return infos;
	}

	public static AccessibilityNodeInfo findAccessibilityNodeInfoByText(AccessibilityNodeInfo root, String text) {
		List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByText(text);
		if (nodes == null) {
			return null;
		}

		for (AccessibilityNodeInfo node : nodes) {
			CharSequence sequence = node.getText();
			if (sequence == null) {
				continue;
			}

			if (text.equals(sequence.toString())) {
				return node;
			}
		}

		return null;
	}

	public static AccessibilityNodeInfo findAccessibilityNodeInfoByViewId(AccessibilityNodeInfo root, String viewId) {
		List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByViewId(viewId);
		if (nodes == null || nodes.isEmpty()) {
			return null;
		}

		return nodes.get(0);
	}

	public static void setAccessibilityNodeSelection(AccessibilityNodeInfo node, int start, int length) {
		if (length > 0) {
			Bundle arguments = new Bundle();
			arguments.putInt(AccessibilityNodeInfo.ACTION_ARGUMENT_SELECTION_START_INT, start);
			arguments.putInt(AccessibilityNodeInfo.ACTION_ARGUMENT_SELECTION_END_INT, start + length);
			node.performAction(AccessibilityNodeInfo.ACTION_SET_SELECTION, arguments);
		} else {
			node.performAction(AccessibilityNodeInfo.ACTION_SELECT);
		}
	}

	public boolean setAccessibilityNodeText(AccessibilityNodeInfo node, String text) {
		node.performAction(AccessibilityNodeInfo.ACTION_FOCUS);

		String oldText = CavanString.fromCharSequence(node.getText());
		if (text.equals(oldText)) {
			return false;
		}

		setAccessibilityNodeSelection(node, 0, oldText.length());
		RedPacketListenerService.postRedPacketCode(this, text);
		node.performAction(AccessibilityNodeInfo.ACTION_PASTE);

		return true;
	}

	public int getRedPacketCodeCount() {
		try {
			if (mService != null) {
				return mService.getCodeCount();
			}
		} catch (RemoteException e) {
			e.printStackTrace();
		}

		return -1;
	}

	public boolean sendRedPacketCode(RedPacketCode code) {
		try {
			if (mService != null) {
				code.setSendPending(false);
				code.setRecvDisable();
				mService.sendRedPacketCode(code.getCode());
				return true;
			}
		} catch (RemoteException e) {
			e.printStackTrace();
		}

		return false;
	}

	public static void dumpAccessibilityNodeInfo(StringBuilder builder, String prefix, AccessibilityNodeInfo node) {
		if (node == null) {
			return;
		}

		builder.append(prefix);
		builder.append(node);
		builder.append('\n');
		prefix += "  ";

		for (int i = 0, count = node.getChildCount(); i < count; i++) {
			dumpAccessibilityNodeInfo(builder, prefix, node.getChild(i));
		}
	}

	public static String dumpAccessibilityNodeInfo(AccessibilityNodeInfo node) {
		StringBuilder builder = new StringBuilder();
		dumpAccessibilityNodeInfo(builder, "", node);
		return builder.toString();
	}

	public static void showAccessibilityNodeInfo(AccessibilityNodeInfo node) {
		CavanAndroid.dLog(dumpAccessibilityNodeInfo(node));
	}

	public static void dumpAccessibilityNodeInfoSimple(StringBuilder builder, String prefix, AccessibilityNodeInfo node) {
		if (node == null) {
			return;
		}

		builder.append(prefix);
		builder.append("├─ ");
		builder.append(node.getClassName());
		builder.append("@");
		builder.append(node.getViewIdResourceName());
		// builder.append(node.hashCode());
		builder.append(": ");
		builder.append(node.getText());
		builder.append('\n');
		prefix += "├──";

		for (int i = 0, count = node.getChildCount(); i < count; i++) {
			dumpAccessibilityNodeInfoSimple(builder, prefix, node.getChild(i));
		}
	}

	public static String dumpAccessibilityNodeInfoSimple(AccessibilityNodeInfo node) {
		StringBuilder builder = new StringBuilder();
		dumpAccessibilityNodeInfoSimple(builder, "", node);
		return builder.toString();
	}

	public static void showAccessibilityNodeInfoSimple(AccessibilityNodeInfo node) {
		CavanAndroid.dLog(dumpAccessibilityNodeInfoSimple(node));
	}

	@Override
	public void onAccessibilityEvent(AccessibilityEvent event) {
		CharSequence sequence = event.getPackageName();
		if (sequence != null) {
			mPackageName = sequence.toString();
		}

		CavanAccessibilityBase accessibility = mAccessibilityMap.get(mPackageName);
		if (accessibility == null) {
			return;
		}

		switch (event.getEventType()) {
		case AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED:
			mWindowStartTime = System.currentTimeMillis();

			sequence = event.getClassName();
			if (sequence != null) {
				mClassName = sequence.toString();
			}

			CavanAndroid.dLog("package = " + mPackageName);
			CavanAndroid.dLog("class = " + mClassName);

			accessibility.onWindowStateChanged(event, mPackageName, mClassName);
			break;

		case AccessibilityEvent.TYPE_WINDOW_CONTENT_CHANGED:
			accessibility.onWindowContentChanged(event);
			break;

		case AccessibilityEvent.TYPE_VIEW_CLICKED:
			if (MainActivity.isListenClickEnabled(CavanAccessibilityService.this)) {
				accessibility.onViewClicked(event);
			}
			break;

		case AccessibilityEvent.TYPE_VIEW_TEXT_CHANGED:
			accessibility.onViewTextChanged(event);
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

		CavanAccessibilityBase accessibility = mAccessibilityMap.get(sequence.toString());
		if (accessibility == null) {
			return false;
		}

		return accessibility.onKeyEvent(event);
	}

	@Override
	protected void onServiceConnected() {
		AccessibilityServiceInfo info = getServiceInfo();

		info.packageNames = PACKAGE_NAMES;

		info.flags |= AccessibilityServiceInfo.DEFAULT |
				AccessibilityServiceInfo.FLAG_REPORT_VIEW_IDS |
				AccessibilityServiceInfo.FLAG_REQUEST_FILTER_KEY_EVENTS |
				AccessibilityServiceInfo.FLAG_RETRIEVE_INTERACTIVE_WINDOWS;

		info.eventTypes = AccessibilityEvent.TYPE_WINDOW_CONTENT_CHANGED |
				AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED | AccessibilityEvent.TYPE_VIEW_CLICKED |
				AccessibilityEvent.TYPE_VIEW_CLICKED | AccessibilityEvent.TYPE_VIEW_TEXT_CHANGED;

		setServiceInfo(info);

		CavanAndroid.dLog("info = " + getServiceInfo());

		super.onServiceConnected();
	}

	@Override
	public void onInterrupt() {
		CavanAndroid.pLog();
	}

	@Override
	public void onCreate() {
		WindowManager manager = (WindowManager) getSystemService(WINDOW_SERVICE);
		if (manager != null) {
			manager.getDefaultDisplay().getSize(mDisplaySize);
		}

		IntentFilter filter = new IntentFilter();
		filter.addAction(MainActivity.ACTION_CODE_TEST);
		filter.addAction(MainActivity.ACTION_CODE_ADD);
		filter.addAction(MainActivity.ACTION_CODE_REMOVE);
		filter.addAction(Intent.ACTION_CLOSE_SYSTEM_DIALOGS);
		filter.addAction(MainActivity.ACTION_UNPACK_MM);
		filter.addAction(MainActivity.ACTION_UNPACK_QQ);

		registerReceiver(mReceiver, filter);

		Intent service = FloatMessageService.startService(this);
		bindService(service, mConnection, 0);

		super.onCreate();
	}

	@Override
	public void onDestroy() {
		unbindService(mConnection);
		unregisterReceiver(mReceiver);

		super.onDestroy();
	}
}
