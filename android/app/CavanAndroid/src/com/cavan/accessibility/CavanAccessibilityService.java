package com.cavan.accessibility;

import java.util.HashMap;
import java.util.HashSet;
import java.util.List;

import android.accessibilityservice.AccessibilityService;
import android.accessibilityservice.AccessibilityServiceInfo;
import android.app.Notification;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Point;
import android.graphics.Rect;
import android.os.Build;
import android.os.Handler;
import android.os.Message;
import android.os.Parcelable;
import android.view.Display;
import android.view.KeyEvent;
import android.view.Surface;
import android.view.WindowManager;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;
import android.view.inputmethod.InputMethodManager;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanKeyguardLock;
import com.cavan.android.CavanThreadedHandler;
import com.cavan.android.CavanWakeLock;
import com.cavan.android.SystemProperties;
import com.cavan.java.CavanJava;
import com.cavan.java.CavanThread;

public class CavanAccessibilityService extends AccessibilityService {

	public static final String ACTION_CODE_RECEIVED = "com.cavan.intent.ACTION_CODE_RECEIVED";

	public static final int POLL_DELAY = 500;
	public static final int LAUNCH_DELAY = 2000;
	public static final int CMD_OVERTIME = 60000;

	private static final int MSG_SCREEN_ON = 1;
	private static final int MSG_SHOW_COUNT_DOWN = 2;
	private static final int MSG_UPDATE_COUNT_DOWN = 3;
	private static final int MSG_SHOW_LOGIN_DIALOG = 4;
	private static final int MSG_SEND_COMMAND = 5;
	private static final int MSG_WAIT_POP_WINDOW = 6;

	public static CavanAccessibilityService instance;

	protected HashMap<String, CavanAccessibilityPackage> mPackages = new HashMap<String, CavanAccessibilityPackage>();
	protected HashSet<AccessibilityNodeInfo> mRecycleNodes = new HashSet<AccessibilityNodeInfo>();
	protected CavanRedPacketList mPackets = new CavanRedPacketList();
	protected CavanAccessibilityPackage mPackage;
	protected boolean mUserPresent = true;
	protected boolean mScreenOn = true;

	private CavanKeyguardLock mKeyguardLock = new CavanKeyguardLock();
	private CavanWakeLock mWakeLock = new CavanWakeLock.WakupLock();
	private CavanRedPacket mPacketDummy = new CavanRedPacket();
	private CavanCountDownDialogBase mCountDownDialog;

	private int mDisplayWidth = 1080;
	private int mDisplayHeight = 1920;

	public class CavanAccessibilityThread extends CavanThread {

		@Override
		public synchronized void wakeup() {
			if (mScreenOn) {
				super.wakeup();
			}
		}

	}

	private CavanAccessibilityThread mPollThread = new CavanAccessibilityThread() {

		public boolean poll(CavanAccessibilityPackage pkg, CavanRedPacket packet) {
			int retry = 0;

			CavanAndroid.dLog("PollThread polling: " + pkg.getName());

			while (mScreenOn && getPendingPacket() == packet) {
				long delay;

				CavanAndroid.dLog("PollThread running");

				AccessibilityNodeInfo root = getRootInActiveWindow();
				if (root == null) {
					CavanAndroid.dLog("Failed to getRootInActiveWindow: " + retry);

					if (++retry > 5) {
						return false;
					}

					delay = POLL_DELAY;
				} else {
					try {
						if (pkg.isCurrentPackage(root)) {
							delay = pkg.poll(packet, root);
							if (delay < 0) {
								pkg.setPending(false);
								return true;
							}

							retry = 0;
						} else {
							CavanAndroid.dLog("Invalid package(" + retry + "): " + root.getPackageName());

							if (++retry > 5) {
								return false;
							}

							packet.launch();
							delay = LAUNCH_DELAY;
						}
					} catch (Exception e) {
						e.printStackTrace();
						return false;
					} finally {
						removeRecycleNodes();
						root.recycle();
					}
				}

				if (getPendingPacket() != packet) {
					break;
				}

				CavanAndroid.dLog("PollThread waitting " + delay);

				if (delay > 0) {
					synchronized (this) {
						try {
							wait(delay);
						} catch (InterruptedException e) {
							e.printStackTrace();
						}
					}
				}
			}

			return true;
		}

		@Override
		public void run() {
			while (true) {
				boolean idle = false;

				while (mScreenOn) {
					CavanAccessibilityPackage pkg;
					CavanRedPacket packet;

					packet = getPendingPacket();
					if (packet == mPacketDummy) {
						pkg = getPendingPackage();
						if (pkg == null) {
							break;
						}

						long delay = pkg.getUnpackDelay();
						if (delay < 0) {
							pkg.setPending(false);
							continue;
						}

						packet.setUnpackDelay(delay);
						packet.setPackage(pkg);
						packet.setGotoIdle(false);
						pkg.setCurrentPacket(null);
					} else {
						packet.setGotoIdle(true);
						pkg = packet.getPackage();
						pkg.setPending(true);
						pkg.setGotoHome(true);
						pkg.setCurrentPacket(packet);
					}

					packet.setPending();
					pkg.onPollStarted();

					int retry = 0;

					while (true) {
						if (poll(pkg, packet)) {
							break;
						}

						CavanAndroid.dLog("Failed to poll: " + retry);

						if (++retry > 3) {
							mPackets.remove(packet);
							break;
						}
					}

					pkg.setCurrentPacket(null);
					pkg.setComplete();
					pkg.onPollStopped();
					idle = packet.getGotoIdle();
				}

				CavanAndroid.dLog("PollThread suspend: idle = " + idle);
				onPollSuspend(idle);

				synchronized (this) {
					try {
						wait();
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}
		}
	};

	public class CavanAccessibilityCommand {

		private long mTime;
		private int mTimes;
		private int mCommand;
		private Object[] mArgs;

		public CavanAccessibilityCommand(int command, Object[] args) {
			mTime = System.currentTimeMillis();
			mCommand = command;
			mArgs = args;
			mTimes = 0;
		}

		public long getTime() {
			return mTime;
		}

		public int getTimes() {
			return mTimes;
		}

		public int getCommand() {
			return mCommand;
		}

		public Object[] getArgs() {
			return mArgs;
		}

		public boolean isTimeout(long timeout) {
			long time = System.currentTimeMillis();
			return time - mTime > timeout;
		}

		public long execute() {
			if (isTimeout(20000)) {
				return -1;
			}

			for (int i = 0; i < 3 && isCurrentCommand(this); i++) {
					AccessibilityNodeInfo root = getRootInActiveWindow();
					if (root == null) {
						return 500;
					}

					mTimes++;

					CavanAccessibilityPackage pkg = getPackage(root);
					if (pkg == null) {
						return doCommand(root, this);
					}

					long delay = pkg.doCommand(root, this);
					if (delay < 0 && pkg.isCommandPending()) {
						mCommandThread.msleep(200);
					} else {
						return delay;
					}
			}

			return 0;
		}
	}

	private CavanAccessibilityCommand mCommand;

	private CavanAccessibilityThread mCommandThread = new CavanAccessibilityThread() {

		@Override
		public void run() {
			while (true) {
				CavanAccessibilityCommand command = mCommand;
				if (command != null) {
					long delay = command.execute();
					CavanAndroid.dLog("delay = " + delay);

					if (delay > 0) {
						synchronized (this) {
							try {
								wait(delay);
							} catch (Exception e) {
								e.printStackTrace();
							}
						}

						continue;
					} else if (delay < 0 && command == mCommand) {
						mCommand = null;
					}
				}

				synchronized (this) {
					try {
						wait();
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}
		}
	};

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			removeMessages(msg.what);

			switch (msg.what) {
			case MSG_SCREEN_ON:
				mPollThread.wakeup();
				break;

			case MSG_SHOW_COUNT_DOWN:
				CavanRedPacket packet = (CavanRedPacket) msg.obj;
				if (packet == null) {
					removeMessages(MSG_UPDATE_COUNT_DOWN);
					break;
				}

				if (mCountDownDialog == null) {
					mCountDownDialog = createCountDownDialog();
					if (mCountDownDialog == null) {
						break;
					}

				}

				mCountDownDialog.show(packet);

			case MSG_UPDATE_COUNT_DOWN:
				CavanCountDownDialogBase dialog = mCountDownDialog;
				if (dialog != null) {
					long delay = dialog.update();
					if (delay > 0) {
						sendEmptyMessageDelayed(MSG_UPDATE_COUNT_DOWN, delay);
					}
				}
				break;

			case MSG_SHOW_LOGIN_DIALOG:
				onShowLoginDialog((CavanAccessibilityPackage) msg.obj);
				break;
			}
		}
	};

	private CavanThreadedHandler mThreadedHandler = new CavanThreadedHandler(CavanAccessibilityService.class) {

		@Override
		public void handleMessage(Message msg) {
			removeMessages(msg.what);

			switch (msg.what) {
			case MSG_SEND_COMMAND:
				if (sendCommand(msg.arg1, (Object[]) msg.obj)) {
					break;
				}

				if (msg.arg2 > 0) {
					postCommand(500, msg.arg2 - 1, msg.arg1, (Object[]) msg.obj);
				}
				break;

			case MSG_WAIT_POP_WINDOW:
				CavanAccessibilityPackage pkg = (CavanAccessibilityPackage) msg.obj;
				AccessibilityNodeInfo root = getRootInActiveWindow();

				if (root == null) {
					postWaitPopWindow(pkg, 50);
				} else if (pkg.isCurrentPackage(root)) {
					try {
						CavanAccessibilityWindow win = pkg.waitPopWindow(root.hashCode());
						if (win != null) {
							CavanAndroid.dLog("win = " + win);
							pkg.onWindowStateChanged(root, win, pkg.touchUpdateTime());
						} else {
							postWaitPopWindow(pkg, 200);
						}
					} catch (Exception e) {
						e.printStackTrace();
					}
				}
				break;
			}
		}
	};

	private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();
			CavanAndroid.dLog("action = " + action);

			switch (action) {
			case Intent.ACTION_SCREEN_ON:
				mScreenOn = true;
				onScreenOn();

				if (mPackets.isEmpty()) {
					break;
				}

				mHandler.sendEmptyMessageDelayed(MSG_SCREEN_ON, 1000);
				acquireScreenLock();
				break;

			case Intent.ACTION_SCREEN_OFF:
				mPackets.clear();
				setPackage(null);
				mUserPresent = false;
				onUserOffline();
				mScreenOn = false;
				onScreenOff();
				releaseScreenLock();
				releaseWakeLock();
				break;

			case Intent.ACTION_USER_PRESENT:
				mUserPresent = true;
				onUserOnline();
				break;

			case Intent.ACTION_CLOSE_SYSTEM_DIALOGS:
				onCloseSystemDialogs(intent.getStringExtra("reason"));
				mPackets.clear();
				setPackage(null);
				break;
			}
		}
	};

	protected void onPollSuspend(boolean idle) {
		if (mScreenOn && idle) {
			CavanAndroid.startLauncher(this);
		}
	}

	public synchronized boolean isCurrentCommand(CavanAccessibilityCommand command) {
		return (command == mCommand);
	}

	public long doCommand(AccessibilityNodeInfo root, CavanAccessibilityCommand command) {
		switch (command.getCommand()) {
		case CavanAccessibilityWindow.CMD_BACK:
			performActionBack();
			break;

		case CavanAccessibilityWindow.CMD_SIGNIN:
			processCommandSignin(root);
			break;
		}

		return 0;
	}

	private boolean processCommandSignin(AccessibilityNodeInfo root) {
		CharSequence pkg = root.getPackageName();
		if (pkg == null) {
			return false;
		}

		Point position = onReadPosition(pkg.toString());
		if (position == null) {
			return false;
		}

		return doInputTap(position.x, position.y);
	}

	public CavanAccessibilityPackage getPendingPackage() {
		synchronized (mPackages) {
			for (CavanAccessibilityPackage pkg : mPackages.values()) {
				if (pkg.isPending()) {
					return pkg;
				}
			}
		}

		return null;
	}

	public CavanRedPacket getPendingPacket() {
		CavanRedPacket packet = mPackets.get();
		if (packet != null) {
			return packet;
		}

		return mPacketDummy;
	}

	protected CavanCountDownDialogBase createCountDownDialog() {
		return null;
	}

	protected void onUserOnline() {}
	protected void onUserOffline() {}
	protected void onScreenOn() {}
	protected void onScreenOff() {}
	protected void onCloseSystemDialogs(String reason) {}

	public Handler getHandler() {
		return mHandler;
	}

	public synchronized boolean isUserPresent() {
		return mUserPresent;
	}

	public synchronized boolean isScreenOn() {
		return mScreenOn;
	}

	public boolean addRecycleNode(AccessibilityNodeInfo node) {
		synchronized (mRecycleNodes) {
			return mRecycleNodes.add(node);
		}
	}

	public int addRecycleNodes(AccessibilityNodeInfo... nodes) {
		int count = 0;

		synchronized (mRecycleNodes) {
			for (AccessibilityNodeInfo node : nodes) {
				if (mRecycleNodes.add(node)) {
					count++;
				}
			}
		}

		return count;
	}

	public int addRecycleNodes(List<AccessibilityNodeInfo> nodes) {
		int count = 0;

		synchronized (mRecycleNodes) {
			for (AccessibilityNodeInfo node : nodes) {
				if (mRecycleNodes.add(node)) {
					count++;
				}
			}
		}

		return count;
	}

	public void removeRecycleNodes() {
		synchronized (mRecycleNodes) {
			try {
				for (AccessibilityNodeInfo node : mRecycleNodes) {
					node.recycle();
				}
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				mRecycleNodes.clear();
			}
		}
	}

	public synchronized CavanAccessibilityPackage getPackage() {
		return mPackage;
	}

	public synchronized boolean setPackage(CavanAccessibilityPackage pkg) {
		if (mPackage == pkg) {
			return false;
		}

		if (mPackage != null) {
			mPackage.onLeave();
		}

		mPackage = pkg;

		if (pkg != null) {
			pkg.onEnter();
		}

		return true;
	}

	public synchronized boolean setPackageByName(CharSequence name) {
		return setPackage(getPackage(name));
	}

	public String getCurrntPacketName() {
		AccessibilityNodeInfo root = getRootInActiveWindow(10);
		if (root == null) {
			return null;
		}

		try {
			CharSequence name = root.getPackageName();
			if (name != null) {
				return name.toString();
			}
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			root.recycle();
		}

		return null;
	}

	public synchronized CavanAccessibilityPackage getCurrentPackage(String name) {
		synchronized (mPackages) {
			return mPackages.get(name);
		}
	}

	public synchronized CavanAccessibilityPackage getCurrentPackage(AccessibilityNodeInfo root) {
		CharSequence name = root.getPackageName();
		if (name == null) {
			return null;
		}

		return getCurrentPackage(name.toString());
	}

	public CavanAccessibilityPackage getCurrentPackage() {
		String name = getCurrntPacketName();
		if (name == null) {
			return null;
		}

		return getCurrentPackage(name);
	}

	public int getDisplayWidth() {
		return mDisplayWidth;
	}

	public int getDisplayHeight() {
		return mDisplayHeight;
	}

	public CavanWakeLock getWakeLock() {
		return mWakeLock;
	}

	public CavanKeyguardLock getKeyguardLock() {
		return mKeyguardLock;
	}

	public void acquireWakeLock() {
		mWakeLock.acquire(this);
	}

	public void acquireWakeLock(long overtime) {
		mWakeLock.acquire(this, overtime);
	}

	public void releaseWakeLock() {
		mWakeLock.release();
	}

	public void acquireScreenLock() {
		mKeyguardLock.acquire(this);
	}

	public void releaseScreenLock() {
		mKeyguardLock.release();
	}

	public void addPackage(CavanAccessibilityPackage pkg) {
		synchronized (mPackages) {
			for (String name : pkg.getNames()) {
				CavanAndroid.dLog("addPackage: " + name);
				mPackages.put(name, pkg);
			}
		}
	}

	public CavanAccessibilityPackage getPackage(CharSequence name) {
		if (name == null) {
			return null;
		}

		synchronized (mPackages) {
			return mPackages.get(name.toString());
		}
	}

	public CavanAccessibilityPackage getPackage(AccessibilityNodeInfo root) {
		return getPackage(root.getPackageName());
	}

	public boolean performActionBack() {
		CavanAndroid.dLog("performActionBack");
		return performGlobalAction(AccessibilityService.GLOBAL_ACTION_BACK);
	}

	public boolean performActionHome() {
		CavanAndroid.dLog("performActionHome");
		return performGlobalAction(AccessibilityService.GLOBAL_ACTION_HOME);
	}

	public void startPoll() {
		mPollThread.wakeup();
	}

	public String[] getPackageNames() {
		String[] packages = new String[mPackages.size()];
		mPackages.keySet().toArray(packages);
		return packages;
	}

	public int getEventTypes() {
		int types = AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED;

		if (SystemProperties.getInt("debug.cavan.dump.click", 0) > 0) {
			types |= AccessibilityEvent.TYPE_VIEW_CLICKED;
		}

		return types;
	}

	public int getEventFlags() {
		return AccessibilityServiceInfo.DEFAULT |
			AccessibilityServiceInfo.FLAG_REPORT_VIEW_IDS |
			AccessibilityServiceInfo.FLAG_REQUEST_FILTER_KEY_EVENTS |
			AccessibilityServiceInfo.FLAG_RETRIEVE_INTERACTIVE_WINDOWS;
	}

	public AccessibilityNodeInfo getRootInActiveWindow(int retry) {
		while (true) {
			AccessibilityNodeInfo root = getRootInActiveWindow();
			if (root != null) {
				return root;
			}

			CavanAndroid.eLog("Failed to getRootInActiveWindow: " + retry);

			if (retry > 0) {
				synchronized (mPollThread) {
					try {
						mPollThread.wait(100);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}

				retry--;
			} else {
				break;
			}
		}

		return null;
	}

	public synchronized void showCountDownView(CavanRedPacket packet) {
		Message message = mHandler.obtainMessage(MSG_SHOW_COUNT_DOWN, packet);
		message.sendToTarget();
	}

	public boolean doShellCommand(String command) {
		return false;
	}

	public boolean doShellCommandF(String format, Object... args) {
		return doShellCommand(String.format(format, args));
	}

	public boolean doInputTap(int x, int y) {
		return doShellCommandF("input tap %d %d", x, y);
	}

	public boolean doInputTap(AccessibilityNodeInfo node) {
		Rect bounds = new Rect();
		node.getBoundsInScreen(bounds);
		CavanAndroid.dLog("bounds = " + bounds);
		return doInputTap(bounds.centerX(), bounds.centerY());
	}

	public boolean doInputTapAndRecycle(AccessibilityNodeInfo node) {
		boolean success = doInputTap(node);
		node.recycle();
		return success;
	}

	public boolean doInputText(String text) {
		return doShellCommandF("input text '%s'", text);
	}

	public boolean doInputKeyEvent(int code) {
		return doShellCommand("input keyevent " + code);
	}

	public boolean doSaveSubject(String question, String answer) {
		return false;
	}

	public boolean doRemoveSubject(String question) {
		return false;
	}

	public boolean doLoadSubjects(HashMap<String, String> subjects) {
		return false;
	}

	public boolean addPacket(CavanAccessibilityPackage pkg, CavanRedPacket packet) {
		acquireWakeLock(2000);

		if (mPackets.add(pkg, packet)) {
			startPoll();
			return true;
		}

		return false;
	}

	public CavanRedPacketList getPackets() {
		return mPackets;
	}

	public void removePacket(CavanRedPacket packet) {
		mPackets.remove(packet);
	}

	public int removePackets(CavanAccessibilityPackage pkg) {
		return mPackets.remove(pkg);
	}

	public int removePackets(CharSequence pkgName) {
		CavanAccessibilityPackage pkg = getPackage(pkgName);
		if (pkg == null) {
			return -1;
		}

		return removePackets(pkg);
	}

	public boolean isInputMethodPicker(AccessibilityNodeInfo root) {
		if (!CavanAccessibilityHelper.isNodePackgeEquals(root, "android")) {
			return false;
		}

		String title = CavanAccessibilityHelper.getChildText(root, 0);
		if (title == null) {
			return false;
		}

		return (title.equals("选择输入法") || title.equals("更改键盘"));
	}

	public boolean setInputMethod(String name) {
		InputMethodManager manager = (InputMethodManager) CavanAndroid.getSystemServiceCached(this, INPUT_METHOD_SERVICE);
		if (manager == null) {
			return false;
		}

		for (int i = 0; i < 3; i++) {
			AccessibilityNodeInfo root = getRootInActiveWindow();

			if (root != null) {
				try {
					if (isInputMethodPicker(root)) {
						AccessibilityNodeInfo node = CavanAccessibilityHelper.findNodeByText(root, name);
						if (node != null) {
							AccessibilityNodeInfo parent = node.getParent();
							node.recycle();

							if (parent != null) {
								return CavanAccessibilityHelper.performClickAndRecycle(parent);
							}
						}
					} else {
						manager.showInputMethodPicker();
					}
				} catch (Exception e) {
					e.printStackTrace();
				} finally {
					root.recycle();
				}
			}

			CavanJava.msleep(500);
		}

		// performActionBack();

		return false;
	}

	public CavanInputMethodService getInputMethodService() {
		CavanInputMethodService ime = CavanInputMethodService.instance;
		if (ime != null) {
			return ime;
		}

		String name = getInputMethodName();
		if (name == null) {
			return null;
		}

		CavanAndroid.dLog("ime = " + name);

		if (setInputMethod(name)) {
			for (int i = 0; i < 10; i++) {
				ime = CavanInputMethodService.instance;
				if (ime != null) {
					return ime;
				}

				CavanJava.msleep(200);
			}
		}

		return null;
	}

	public boolean setInputMethod() {
		return (getInputMethodService() != null);
	}

	protected String getInputMethodName() {
		return null;
	}

	public void postCommand(long delay, int retry, int command, Object... args) {
		Message message = mThreadedHandler.obtainMessage(MSG_SEND_COMMAND, command, retry, args);
		mThreadedHandler.sendMessageDelayed(message, delay);
	}

	public void postCommand() {
		if (mCommand != null) {
			mCommandThread.wakeup();
		}
	}

	public void stopCommand() {
		mCommand = null;
	}

	public boolean sendCommand(int command, Object... args) {
		mCommand = new CavanAccessibilityCommand(command, args);
		mCommandThread.wakeup();
		return true;
	}

	public boolean sendCommand(int command) {
		return sendCommand(command, (Object[]) null);
	}

	public boolean sendText(String message, boolean commit) {
		if (sendCommand(CavanAccessibilityWindow.CMD_SEND_TEXT, message, commit)) {
			return true;
		}

		AccessibilityNodeInfo root = getRootInActiveWindow();
		if (root == null) {
			return false;
		}

		try {
			AccessibilityNodeInfo node = root.findFocus(AccessibilityNodeInfo.FOCUS_INPUT);
			if (node != null) {
				boolean success = (CavanAccessibilityHelper.setNodeText(this, node, message) != null);
				node.recycle();
				return success;
			}
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			root.recycle();
		}

		return false;
	}

	public boolean login(String username, String password) {
		return sendCommand(CavanAccessibilityWindow.CMD_LOGIN, username, password);
	}

	public boolean signin() {
		return sendCommand(CavanAccessibilityWindow.CMD_SIGNIN);
	}

	public boolean follow() {
		return sendCommand(CavanAccessibilityWindow.CMD_FOLLOW);
	}

	public boolean unfollow() {
		return sendCommand(CavanAccessibilityWindow.CMD_UNFOLLOW);
	}

	public boolean sendCommandShred(int mode) {
		return sendCommand(CavanAccessibilityWindow.CMD_SHARE, mode);
	}

	public boolean refresh() {
		return sendCommand(CavanAccessibilityWindow.CMD_REFRESH);
	}

	public boolean sendCommandBack() {
		if (sendCommand(CavanAccessibilityWindow.CMD_BACK)) {
			return true;
		}

		return performActionBack();
	}

	public boolean sendCommandHome() {
		return sendCommand(CavanAccessibilityWindow.CMD_HOME);
	}

	public boolean sendCommandWeb(String action) {
		return sendCommand(CavanAccessibilityWindow.CMD_WEB, action);
	}

	public boolean savePosition(Point point) {
		String pkg = getCurrntPacketName();
		if (pkg == null) {
			return false;
		}

		return onSavePosition(pkg, point);
	}

	public boolean removePosition() {
		String pkg = getCurrntPacketName();
		if (pkg == null) {
			return false;
		}

		return onRemovePosition(pkg);
	}

	public Point readPosition() {
		String pkg = getCurrntPacketName();
		if (pkg == null) {
			return null;
		}

		return onReadPosition(pkg);
	}

	protected boolean onSavePosition(String pkg, Point point) {
		return false;
	}

	protected Point onReadPosition(String pkg) {
		return null;
	}

	protected boolean onRemovePosition(String pkg) {
		return false;
	}

	public Class<?> getBroadcastReceiverClass() {
		return mBroadcastReceiver.getClass();
	}

	public void setEventTypes(int types) {
		AccessibilityServiceInfo info = getServiceInfo();
		if (info != null && info.eventTypes != types) {
			info.eventTypes = types;
			setServiceInfo(info);
		}
	}

	public boolean dump(boolean simple) {
		AccessibilityNodeInfo root = getRootInActiveWindow();
		if (root == null) {
			return false;
		}

		CavanAccessibilityHelper.dumpNode(root, simple);
		root.recycle();

		return true;
	}

	public AccessibilityNodeInfo getChildRecursive(Object... indexs) {
		AccessibilityNodeInfo root = getRootInActiveWindow(3);
		if (root == null) {
			return null;
		}

		if (indexs.length < 1) {
			return root;
		}

		try {
			return CavanAccessibilityHelper.getChildRecursive(root, indexs);
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			root.recycle();
		}

		return null;
	}

	@Override
	protected void onServiceConnected() {
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
			AccessibilityServiceInfo info = getServiceInfo();
			info.packageNames = getPackageNames();
			info.eventTypes = getEventTypes();
			info.flags = getEventFlags();
			setServiceInfo(info);
			CavanAndroid.dLog("info = " + getServiceInfo());
		}

		super.onServiceConnected();
	}

	public void onAccessibilityEvent(AccessibilityEvent event, AccessibilityNodeInfo root) {
		CavanAccessibilityPackage pkg = getPackage(event.getPackageName());
		if (pkg != null) {
			pkg.onAccessibilityEvent(root, event);
		} else {
			switch (event.getEventType()) {
			case AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED:
				onWindowStateChanged(root, event);
				break;

			case AccessibilityEvent.TYPE_WINDOW_CONTENT_CHANGED:
				onWindowContentChanged(root, event);
				break;

			case AccessibilityEvent.TYPE_VIEW_CLICKED:
				onViewClicked(root, event);
				break;

			case AccessibilityEvent.TYPE_VIEW_TEXT_CHANGED:
				onViewTextChanged(root, event);
				break;

			case AccessibilityEvent.TYPE_NOTIFICATION_STATE_CHANGED:
				Parcelable data = event.getParcelableData();
				if (data instanceof Notification) {
					onNotificationStateChanged(root, (Notification) data);
				}
				break;

			default:
				CavanAndroid.wLog("onAccessibilityEvent: " + event);
			}
		}
	}

	@Override
	public void onAccessibilityEvent(AccessibilityEvent event) {
		AccessibilityNodeInfo root = getRootInActiveWindow();
		if (root != null) {
			try {
				onAccessibilityEvent(event, root);
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				root.recycle();
			}
		} else if (event.getEventType() == AccessibilityEvent.TYPE_NOTIFICATION_STATE_CHANGED) {
			try {
				onAccessibilityEvent(event, null);
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	protected void onWindowStateChanged(AccessibilityNodeInfo root, AccessibilityEvent event) {
		CavanAndroid.dLog("onWindowStateChanged: " + event);
	}

	protected void onWindowContentChanged(AccessibilityNodeInfo root, AccessibilityEvent event) {
		CavanAndroid.dLog("onWindowContentChanged: " + event);
	}

	protected void onViewClicked(AccessibilityNodeInfo root, AccessibilityEvent event) {
		CavanAndroid.dLog("onViewClicked: " + event);

		AccessibilityNodeInfo source = event.getSource();
		CavanAndroid.dLog("source = " + source);

		if (source != null) {
			source.recycle();
		}
	}

	protected void onViewTextChanged(AccessibilityNodeInfo root, AccessibilityEvent event) {
		CavanAndroid.dLog("onViewTextChanged: " + event);
	}

	protected void onNotificationStateChanged(AccessibilityNodeInfo root, Notification data) {
		CavanAndroid.dLog("onNotificationStateChanged: " + data);
	}

	@Override
	protected boolean onKeyEvent(KeyEvent event) {
		AccessibilityNodeInfo root = getRootInActiveWindow();
		if (root != null) {
			try {
				if (event.getKeyCode() == KeyEvent.KEYCODE_VOLUME_UP && event.getAction() == KeyEvent.ACTION_UP) {
					int dump = SystemProperties.getInt("debug.cavan.dump.node", 0);
					if (dump > 0) {
						CavanAccessibilityHelper.dumpNode(root, dump < 2);
					}
				}

				CavanAccessibilityPackage pkg = getCurrentPackage(root);
				if (pkg != null) {
					pkg.onKeyEvent(pkg, root, event);
				}
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				root.recycle();
			}
		}

		return super.onKeyEvent(event);
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
				Display display = manager.getDefaultDisplay();
				int rotation = display.getRotation();
				Point point = new Point();

				display.getRealSize(point);

				CavanAndroid.dLog("rotation = " + rotation);
				CavanAndroid.dLog("size = " + point);

				if (rotation == Surface.ROTATION_0 || rotation == Surface.ROTATION_180) {
					mDisplayWidth = point.x;
					mDisplayHeight = point.y;
				} else {
					mDisplayWidth = point.y;
					mDisplayHeight = point.x;
				}
			}
		}

		CavanAndroid.dLog("mDisplayWidth = " + mDisplayWidth);
		CavanAndroid.dLog("mDisplayHeight = " + mDisplayHeight);

		for (CavanAccessibilityPackage pkg : mPackages.values()) {
			pkg.onCreate();
		}

		IntentFilter filter = new IntentFilter();
		filter.addAction(Intent.ACTION_SCREEN_OFF);
		filter.addAction(Intent.ACTION_SCREEN_ON);
		filter.addAction(Intent.ACTION_USER_PRESENT);
		filter.addAction(Intent.ACTION_CLOSE_SYSTEM_DIALOGS);
		registerReceiver(mBroadcastReceiver, filter);

		instance = this;
	}

	@Override
	public void onDestroy() {
		instance = null;

		unregisterReceiver(mBroadcastReceiver);

		for (CavanAccessibilityPackage pkg : mPackages.values()) {
			pkg.onDestroy();
		}

		super.onDestroy();
	}

	public void showLoginDialog(CavanAccessibilityPackage pkg) {
		CavanAndroid.dLog("showLoginDialog");
		mHandler.obtainMessage(MSG_SHOW_LOGIN_DIALOG, pkg).sendToTarget();
	}

	public boolean onShowLoginDialog(CavanAccessibilityPackage pkg) {
		CavanAndroid.dLog("onShowLoginDialog");
		return false;
	}

	public String getPassword(CavanAccessibilityPackage pkg, String username) {
		CavanAndroid.dLog("getPassword: " + username);
		return null;
	}

	public AccessibilityNodeInfo findFocusInput() {
		return findFocus(AccessibilityNodeInfo.FOCUS_INPUT);
	}

	public AccessibilityNodeInfo findFocusAccessibility() {
		return findFocus(AccessibilityNodeInfo.FOCUS_ACCESSIBILITY);
	}

	public void postWaitPopWindow(CavanAccessibilityPackage pkg, int delay) {
		Message msg = mThreadedHandler.obtainMessage(MSG_WAIT_POP_WINDOW, pkg);
		mThreadedHandler.sendMessageDelayed(msg, delay);
	}

	public boolean isInformationGroup(String chatting) {
		return false;
	}
}
