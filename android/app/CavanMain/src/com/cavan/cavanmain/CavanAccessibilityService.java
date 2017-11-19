package com.cavan.cavanmain;

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
import android.os.Build;
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

import com.cavan.android.CavanAccessibility;
import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;
import com.cavan.android.SystemProperties;
import com.cavan.java.CavanString;

public class CavanAccessibilityService extends AccessibilityService {

	public static boolean sBootComplete;

	private static final int MSG_CHECK_CONTENT = 1;
	private static final int MSG_CHECK_AUTO_OPEN_APP = 2;
	private static final int MSG_SELECT_INPUT_METHOD = 3;
	private static final int MSG_PERFORM_GLOBAL_ACTION = 4;
	private static final int MSG_BOOT_COMPLETED = 5;

	private static final String[] PACKAGE_NAMES = {
		CavanPackageName.QQ,
		CavanPackageName.MM,
		CavanPackageName.ALIPAY,
		CavanPackageName.SOGOU_IME,
		CavanPackageName.SOGOU_OCR,
	};

	private static CavanAccessibilityService sInstance;

	public static CavanAccessibilityService getInstance() {
		return sInstance;
	}

	private Dialog mCheckContentDialog;

	private long mWindowStartTime;
	private String mClassName = CavanString.EMPTY_STRING;
	private String mPackageName = CavanString.EMPTY_STRING;

	private CavanAccessibilityQQ mAccessibilityQQ = new CavanAccessibilityQQ(this);
	private CavanAccessibilityMM mAccessibilityMM = new CavanAccessibilityMM(this);
	private CavanAccessibilitySogou mAccessibilitySogou = new CavanAccessibilitySogou(this);
	private CavanAccessibilityAlipay mAccessibilityAlipay = new CavanAccessibilityAlipay(this);
	private HashMap<String, CavanAccessibilityBase<?>> mAccessibilityMap = new HashMap<String, CavanAccessibilityBase<?>>();

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
			case Intent.ACTION_CLOSE_SYSTEM_DIALOGS:
				mAccessibilityAlipay.setAutoOpenAlipayEnable(false);
				CavanAndroid.dLog("reason = " + intent.getStringExtra("reason"));
				break;

			case CavanMessageActivity.ACTION_BOOT_COMPLETED:
				mHandler.sendEmptyMessage(MSG_BOOT_COMPLETED);
				break;
			}
		}
	};

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			removeMessages(msg.what);

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
				builder.setPositiveButton(R.string.send, new OnClickListener() {

					@Override
					public void onClick(DialogInterface dialog, int which) {
						String text = editText.getText().toString();

						setAutoOpenAppEnable(true);

						RedPacketListenerService listener = RedPacketListenerService.getInstance();

						if (listener != null) {
							if (checkBox != null && checkBox.isChecked()) {
								if (text != null) {
									for (String line : text.split("\n")) {
										String code = RedPacketCode.filtration(line);

										if (code.length() > 0) {
											listener.addRedPacketCode(code, "图片识别", false);
										}
									}
								}
							} else {
								listener.addRedPacketContent(null, text, "图片识别", false, true, 1);
							}
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
				if (needDisableAutoOpenApp()) {
					CavanMessageActivity.setAutoOpenAppEnable(false);
					sendEmptyMessageDelayed(MSG_CHECK_AUTO_OPEN_APP, 2000);
				} else {
					CavanMessageActivity.setAutoOpenAppEnable(true);
				}
				break;

			case MSG_SELECT_INPUT_METHOD:
				setInputMethod((String) msg.obj, msg.arg1);
				break;

			case MSG_PERFORM_GLOBAL_ACTION:
				CavanAndroid.dLog("performGlobalAction: " + msg.arg1);
				performGlobalAction(msg.arg1);
				break;

			case MSG_BOOT_COMPLETED:
				CavanAndroid.dLog("MSG_BOOT_COMPLETE");
				sBootComplete = false;
				if (Build.MODEL.equals("ZTE Q301C")) {
					Message message = obtainMessage(MSG_PERFORM_GLOBAL_ACTION);
					message.arg1 = GLOBAL_ACTION_BACK;
					sendMessageDelayed(message, 20000);
				}
				break;
			}
		}
	};

	public CavanAccessibilityService() {
		super();

		mAccessibilityMap.put(CavanPackageName.QQ, mAccessibilityQQ);
		mAccessibilityMap.put(CavanPackageName.MM, mAccessibilityMM);
		mAccessibilityMap.put(CavanPackageName.ALIPAY, mAccessibilityAlipay);
		mAccessibilityMap.put(CavanPackageName.SOGOU_IME, mAccessibilitySogou);
		mAccessibilityMap.put(CavanPackageName.SOGOU_OCR, mAccessibilitySogou);
	}

	public CavanAccessibilityQQ getAccessibilityQQ() {
		return mAccessibilityQQ;
	}

	public CavanAccessibilityMM getAccessibilityMM() {
		return mAccessibilityMM;
	}

	public CavanAccessibilitySogou getAccessibilitySogou() {
		return mAccessibilitySogou;
	}

	public CavanAccessibilityAlipay getAccessibilityAlipay() {
		return mAccessibilityAlipay;
	}

	public void setAutoOpenAppEnable(boolean enable) {
		if (enable) {
			mHandler.removeMessages(MSG_CHECK_AUTO_OPEN_APP);
			CavanMessageActivity.setAutoOpenAppEnable(true);
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

	public boolean startNextPendingActivity() {
		if (getMessageCount() > 0) {
			if (CavanMessageActivity.isAutoOpenAppEnabled(this)) {
				for (CavanAccessibilityBase<?> node : mAccessibilityMap.values()) {
					if (node.getPacketCount() > 0) {
						CavanAndroid.startActivity(this, node.getPackageName());
						return true;
					}
				}
			}
		} else {
			for (CavanAccessibilityBase<?> node : mAccessibilityMap.values()) {
				node.clearPackets();
			}
		}

		return false;
	}

	public boolean startIdleActivity() {
		if (CavanMessageActivity.isAutoBackDesktopEnabled(this)) {
			performGlobalAction(AccessibilityService.GLOBAL_ACTION_HOME);
			return true;
		} else if (CavanAndroid.startActivity(this, CavanPackageName.ALIPAY)) {
			return true;
		} else {
			return CavanAndroid.startActivity(this, CavanMessageActivity.class);
		}
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
				code.setRecvEnable(false);
				mService.sendRedPacketCode(code.getCode());
				return true;
			}
		} catch (RemoteException e) {
			e.printStackTrace();
		}

		return false;
	}

	public int getMessageCount() {
		int count = 0;

		if (mService != null) {
			try {
				count = mService.getMessageCount();
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}

		CavanAndroid.dLog("getMessageCount = " + count);

		return count;
	}

	public boolean setInputMethod(String name, int retry) {
		AccessibilityNodeInfo root = getRootInActiveWindow();
		CavanAccessibility.dumpNodeSimple(root);
		if (root != null && "android".equals(root.getPackageName())) {
			AccessibilityNodeInfo node = CavanAccessibility.findNodeByText(root, name);
			if (node != null) {
				AccessibilityNodeInfo parent = node.getParent();
				node.recycle();

				if (parent != null) {
					CavanAccessibility.performClickAndRecycle(parent);
					return true;
				}

				return false;
			} else if (CavanAccessibility.getNodeCountByTexts(root, "选择输入法") > 0) {
				return false;
			}
		}

		if (retry > 0) {
			CavanAndroid.showInputMethodPicker(this);

			Message message = mHandler.obtainMessage(MSG_SELECT_INPUT_METHOD, name);
			message.arg1 = retry - 1;
			mHandler.sendMessageDelayed(message, 500);
		}

		return false;
	}

	@Override
	public void onAccessibilityEvent(AccessibilityEvent event) {
		boolean dump = CavanAccessibility.dumpEvent(event, "debug.cavan.dump.event");

		CharSequence sequence = event.getPackageName();
		if (sequence != null) {
			mPackageName = sequence.toString();
		}

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

		case AccessibilityEvent.TYPE_VIEW_CLICKED:
			if (!dump) {
				CavanAccessibility.dumpEvent(event, "debug.cavan.dump.click");
			}

			if (CavanMessageActivity.isListenClickEnabled(CavanAccessibilityService.this)) {
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

		if (event.getKeyCode() == KeyEvent.KEYCODE_VOLUME_UP) {
			int dump = SystemProperties.getInt("debug.cavan.dump.node", 0);
			if (dump > 0) {
				if (event.getAction() == KeyEvent.ACTION_UP) {
					if (dump > 1) {
						CavanAccessibility.dumpNode(root);
					} else {
						CavanAccessibility.dumpNodeSimple(root);
					}
				}

				// return true;
			}
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
		}

		super.onServiceConnected();
	}

	@Override
	public void onInterrupt() {
		CavanAndroid.pLog();
	}

	@Override
	public void onCreate() {
		super.onCreate();

		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.ICE_CREAM_SANDWICH) {
			WindowManager manager = (WindowManager) getSystemService(WINDOW_SERVICE);
			if (manager != null) {
				manager.getDefaultDisplay().getSize(mDisplaySize);
			}
		}

		IntentFilter filter = new IntentFilter();
		filter.addAction(Intent.ACTION_CLOSE_SYSTEM_DIALOGS);
		filter.addAction(CavanMessageActivity.ACTION_BOOT_COMPLETED);

		registerReceiver(mReceiver, filter);

		Intent service = FloatMessageService.startService(this);
		bindService(service, mConnection, 0);

		if (sBootComplete) {
			mHandler.sendEmptyMessage(MSG_BOOT_COMPLETED);
		}

		sInstance = this;
	}

	@Override
	public void onDestroy() {
		sInstance = null;

		Intent intent = new Intent(CavanMessageActivity.ACTION_SERVICE_EXIT);
		intent.putExtra("service", getClass().getCanonicalName());
		sendBroadcast(intent);

		unbindService(mConnection);
		unregisterReceiver(mReceiver);

		super.onDestroy();
	}

	public static boolean checkAndOpenSettingsActivity(Context context) {
		if (CavanAndroid.isAccessibilityServiceEnabled(context, CavanAccessibilityService.class)) {
			return true;
		}

		PermissionSettingsActivity.startAccessibilitySettingsActivity(context);

		return false;
	}
}
