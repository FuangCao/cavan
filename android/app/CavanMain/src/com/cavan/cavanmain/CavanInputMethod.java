package com.cavan.cavanmain;

import java.io.IOException;
import java.io.InputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.List;

import android.app.ActivityManager;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.inputmethodservice.InputMethodService;
import android.inputmethodservice.Keyboard;
import android.inputmethodservice.KeyboardView;
import android.inputmethodservice.KeyboardView.OnKeyboardActionListener;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.GridView;

import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanJava;
import com.cavan.java.CavanString;

public class CavanInputMethod extends InputMethodService implements OnClickListener, OnKeyboardActionListener {

	public static final int CODE_MAX_COLUMNS = 4;

	public static final int KEYCODE_COPY = 1;
	public static final int KEYCODE_PASTE = 2;
	public static final int KEYCODE_CUT = 3;
	public static final int KEYCODE_SELECT = 4;
	public static final int KEYCODE_SELECT_ALL = 5;
	public static final int KEYCODE_LEFT = 6;
	public static final int KEYCODE_RIGHT = 7;
	public static final int KEYCODE_INPUT_METHOD = 8;
	public static final int KEYCODE_DELETE = 9;
	public static final int KEYCODE_CLEAR = 10;
	public static final int KEYCODE_DONE = 11;
	public static final int KEYCODE_ENTER = 12;
	public static final int KEYCODE_SPACE = 13;

	private static final int MSG_POST_CODE = 1;

	private int mCodeIndex;
	private boolean mCodePending;
	private int mActivityRepeat;
	private String mActivityClassName;

	private List<RedPacketCode> mCodes;
	private GridView mCodeGridView;

	private Keyboard mKeyboard;
	private KeyboardView mKeyboardView;
	private InputMethodManager mManager;

	private boolean mIsAlipay;
	private boolean mSelectioActive;
	private int mSelection;
	private int mSelectionStart;
	private int mSelectionEnd;

	private ServerSocket mServerSocket;
	private Socket mClientSocket;

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case MSG_POST_CODE:
				autoSendRedPacketCode();
				break;
			}
		}
	};

	private BroadcastReceiver mReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();

			CavanAndroid.eLog("action = " + action);

			if (action.equals(FloatMessageService.ACTION_CODE_UPDATED)) {
				updateData();
			}
		}
	};

	private BaseAdapter mAdapter = new BaseAdapter() {

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			Button view = new Button(CavanInputMethod.this);
			view.setOnClickListener(CavanInputMethod.this);
			view.setText(mCodes.get(position).getCode());
			return view;
		}

		@Override
		public long getItemId(int position) {
			return 0;
		}

		@Override
		public Object getItem(int position) {
			return null;
		}

		@Override
		public int getCount() {
			if (mCodes != null) {
				return mCodes.size();
			}

			return 0;
		}
	};

	private IFloatMessageService mService;
	private ServiceConnection mConnection = new ServiceConnection() {

		@Override
		public void onServiceDisconnected(ComponentName name) {
			mService = null;
		}

		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			mService = IFloatMessageService.Stub.asInterface(service);
			updateData();
		}
	};

	public void updateData() {
		if (mService != null) {
			try {
				int columns;

				mCodes = mService.getCodes();
				if (mCodes == null) {
					columns = 0;
				} else {
					columns = mCodes.size();
					if (columns > CODE_MAX_COLUMNS) {
						int max = 0;
						int size = columns;

						for (int i = CODE_MAX_COLUMNS; i > 1; i--) {
							int remain = size % i;
							if (remain == 0) {
								columns = i;
								break;
							}

							if (remain >= max) {
								columns = i;
								max = remain;
							}
						}
					}
				}

				if (mCodeGridView != null) {
					mCodeGridView.setNumColumns(columns);
					mAdapter.notifyDataSetChanged();
				}
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}

		autoSendRedPacketCode();
	}

	@SuppressWarnings("deprecation")
	public void autoSendRedPacketCode(RedPacketCode code) {
		ActivityManager manager = (ActivityManager) getSystemService(ACTIVITY_SERVICE);
		ComponentName info = manager.getRunningTasks(1).get(0).topActivity;
		String pkgName = info.getPackageName();
		String clsName = info.getClassName();
		long delayMillis = 500;

		CavanAndroid.eLog("activity = " + clsName);
		CavanAndroid.eLog(code.toString());

		if (!clsName.equals(mActivityClassName)) {
			mActivityClassName = clsName;
			mActivityRepeat = 0;
		}

		if (pkgName.equals("com.eg.android.AlipayGphone")) {
			CavanAndroid.eLog("mActivityRepeat = " + mActivityRepeat);

			if (mActivityRepeat < 100) {
				if (clsName.equals("com.alipay.android.phone.discovery.envelope.HomeActivity")) {
					if (mActivityRepeat < 3) {
						CavanAndroid.pfLog("index = %d, code = %s", mCodeIndex, code);

						mCodePending = true;
						sendRedPacketCode(code.getCode());
					}
				} else if (clsName.equals("com.alipay.android.phone.discovery.envelope.get.GetRedEnvelopeActivity")) {
					if (mCodePending) {
						int delay = (int) (code.getDelay() / 1000);
						if (delay > 0) {
							CavanAndroid.showToast(this, String.format("%d 秒后自动提交", delay));
							delayMillis = 1000;
						} else {
							sendKeyDownUp(KeyEvent.KEYCODE_DPAD_DOWN);
							sendKeyDownUp(KeyEvent.KEYCODE_ENTER);
						}
					} else {
						sendDownUpKeyEvents(KeyEvent.KEYCODE_BACK);
					}
				} else if ((mActivityRepeat & 1) == 0) {
					if (clsName.equals("com.alipay.mobile.nebulacore.ui.H5Activity") ||
							clsName.equals("com.alipay.android.phone.discovery.envelope.mine.AllCouponList")) {
						sendDownUpKeyEvents(KeyEvent.KEYCODE_BACK);
					} else if (clsName.equals("com.alipay.android.phone.discovery.envelope.crowd.CrowdHostActivity")) {
						if (mCodePending) {
							mCodePending = false;
							mCodeIndex++;
						}

						sendDownUpKeyEvents(KeyEvent.KEYCODE_BACK);
					}
				}

				mActivityRepeat++;
			}
		}

		mHandler.removeMessages(MSG_POST_CODE);
		mHandler.sendEmptyMessageDelayed(MSG_POST_CODE, delayMillis);
	}

	public void setCodeIndex(int index) {
		mCodeIndex = index;
		mCodePending = false;
		mActivityClassName = null;
	}

	public void autoSendRedPacketCode() {
		if (mCodes == null) {
			setCodeIndex(0);
		} else if (mCodeIndex < mCodes.size()) {
			autoSendRedPacketCode(mCodes.get(mCodeIndex));
		} else {
			setCodeIndex(mCodes.size());
		}
	}

	public void sendFinishAction(InputConnection conn) {
		int action = EditorInfo.IME_ACTION_GO;
		EditorInfo info = getCurrentInputEditorInfo();

		if (info != null && info.actionLabel != null) {
			action = info.actionId;
		}

		conn.performEditorAction(action);
	}

	public boolean sendRedPacketCode(CharSequence code) {
		InputConnection conn = getCurrentInputConnection();
		if (conn == null) {
			return false;
		}

		conn.performContextMenuAction(android.R.id.selectAll);
		conn.commitText(code, 0);
		sendFinishAction(conn);

		return true;
	}

	public boolean sendKeyEvent(int code, int value) {
		InputConnection conn = getCurrentInputConnection();
		if (conn == null) {
			return false;
		}

		long time = System.currentTimeMillis();
		int action = (value == 0) ? KeyEvent.ACTION_UP : KeyEvent.ACTION_DOWN;
		KeyEvent event = new KeyEvent(time, time, action, code, 0, 0, KeyCharacterMap.VIRTUAL_KEYBOARD, 0, 0);

		return conn.sendKeyEvent(event);
	}

	public boolean sendKeyDownUp(int code) {
		return sendKeyEvent(code, 1) && sendKeyEvent(code, 0);
	}

	@Override
	public void onCreate() {
		mManager = (InputMethodManager) getSystemService(INPUT_METHOD_SERVICE);

		Intent service = FloatMessageService.startService(this);
		bindService(service, mConnection, 0);

		registerReceiver(mReceiver, new IntentFilter(FloatMessageService.ACTION_CODE_UPDATED));

		KeypadThread thread = new KeypadThread();
		thread.start();

		super.onCreate();
	}

	@Override
	public void onDestroy() {
		if (mClientSocket != null) {
			try {
				mClientSocket.close();
				mClientSocket = null;
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		if (mServerSocket != null) {
			try {
				mServerSocket.close();
				mServerSocket = null;
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		unregisterReceiver(mReceiver);
		unbindService(mConnection);

		super.onDestroy();
	}

	@Override
	public void onUpdateSelection(int oldSelStart, int oldSelEnd,
			int newSelStart, int newSelEnd, int candidatesStart, int candidatesEnd) {
		if (newSelStart < newSelEnd) {
			mSelectioActive = true;
		} else {
			mSelection = newSelStart;

			if (newSelStart == 8 && mIsAlipay) {
				InputConnection conn = getCurrentInputConnection();
				if (conn != null) {
					CharSequence text = conn.getTextBeforeCursor(8, 0);
					if (text != null && text.length() == 8 && CavanJava.isDigit(text)) {
						sendFinishAction(conn);
					}
				}
			}
		}

		mSelectionStart = newSelStart;
		mSelectionEnd = newSelEnd;

		super.onUpdateSelection(oldSelStart, oldSelEnd, newSelStart, newSelEnd,
				candidatesStart, candidatesEnd);
	}

	@Override
	public void onStartInputView(EditorInfo info, boolean restarting) {
		String pkgName = getCurrentInputEditorInfo().packageName;
		mIsAlipay = RedPacketNotification.PACKAGE_NAME_ALIPAY.equals(pkgName);

		if (mIsAlipay && mCodes != null && mCodes.size() == 1) {
			sendRedPacketCode(mCodes.get(0).getCode());
		}

		super.onStartInputView(info, restarting);
	}

	@Override
	public View onCreateInputView() {
		View view = View.inflate(this, R.layout.keyboard, null);

		mCodeGridView = (GridView) view.findViewById(R.id.gridViewCodes);
		mCodeGridView.setAdapter(mAdapter);

		mKeyboardView = (KeyboardView) view.findViewById(R.id.keyboardView);
		mKeyboard = new Keyboard(this, R.xml.keyboard);
		mKeyboardView.setKeyboard(mKeyboard);
		mKeyboardView.setEnabled(true);
		mKeyboardView.setPreviewEnabled(false);
		mKeyboardView.setOnKeyboardActionListener(this);

		return view;
	}

	@Override
	public void onClick(View v) {
		Button button = (Button) v;
		sendRedPacketCode(button.getText());
	}

	@Override
	public void onPress(int primaryCode) {
	}

	@Override
	public void onRelease(int primaryCode) {
	}

	@Override
	public void onKey(int primaryCode, int[] keyCodes) {
		InputConnection conn = getCurrentInputConnection();
		if (conn == null) {
			return;
		}

		switch (primaryCode) {
		case KEYCODE_DELETE:
			CharSequence text = conn.getSelectedText(0);
			if (text != null && text.length() > 0) {
				conn.commitText(CavanString.EMPTY_STRING, 1);
			} else {
				conn.deleteSurroundingText(1, 0);
			}

			mSelectioActive = false;
			break;

		case KEYCODE_CLEAR:
			conn.performContextMenuAction(android.R.id.selectAll);
			conn.commitText(CavanString.EMPTY_STRING, 0);
			break;

		case KEYCODE_DONE:
			sendFinishAction(conn);
			break;

		case KEYCODE_COPY:
			conn.performContextMenuAction(android.R.id.copy);
			break;

		case KEYCODE_CUT:
			mSelectioActive = false;
			conn.performContextMenuAction(android.R.id.cut);
			break;

		case KEYCODE_PASTE:
			mSelectioActive = false;
			conn.performContextMenuAction(android.R.id.paste);
			break;

		case KEYCODE_SELECT_ALL:
			mSelectioActive = true;
			conn.performContextMenuAction(android.R.id.selectAll);
			break;

		case KEYCODE_INPUT_METHOD:
			mManager.showInputMethodPicker();
			break;

		case KEYCODE_SELECT:
			if (mSelectioActive) {
				mSelectioActive = false;
				conn.setSelection(mSelection, mSelection);
			} else {
				mSelectioActive = true;
			}
			break;

		case KEYCODE_LEFT:
			if (mSelectioActive) {
				if (mSelection > mSelectionStart) {
					conn.setSelection(mSelectionStart, --mSelection);
				} else if (mSelection > 0) {
					conn.setSelection(--mSelection, mSelectionEnd);
				}

				conn.setSelection(mSelection, mSelectionEnd);
			} else if (mSelection > 0) {
				conn.setSelection(mSelection - 1, mSelection - 1);
			}
			break;

		case KEYCODE_RIGHT:
			if (mSelectioActive) {
				if (mSelection > mSelectionStart) {
					conn.setSelection(mSelectionStart, ++mSelection);
				} else {
					conn.setSelection(++mSelection, mSelectionEnd);
				}
			} else {
				conn.setSelection(mSelection + 1, mSelection + 1);
			}
			break;
		}
	}

	@Override
	public void onText(CharSequence text) {
		mSelectioActive = false;
		getCurrentInputConnection().commitText(text, 1);
	}

	@Override
	public void swipeLeft() {
	}

	@Override
	public void swipeRight() {
	}

	@Override
	public void swipeDown() {
	}

	@Override
	public void swipeUp() {
	}

	public class KeypadThread extends Thread {

		@Override
		public void run() {
			try {
				mServerSocket = new ServerSocket(23456);
				CavanAndroid.eLog("server = " + mServerSocket);

				while (mServerSocket != null) {
					mClientSocket = mServerSocket.accept();
					if (mClientSocket == null) {
						break;
					}

					CavanAndroid.eLog("client = " + mClientSocket);

					try {
						InputStream stream = mClientSocket.getInputStream();

						byte[] bytes = new byte[1024];

						while (true) {
							int length = stream.read(bytes);
							if (length <= 0) {
								break;
							}

							for (int i = 0; i < length; i++) {
								int code = bytes[i] & 0xFF;

								CavanAndroid.eLog("code = " + code);
								sendKeyDownUp(code);
							}
						}
					} catch (Exception e) {
						e.printStackTrace();
					} finally {
						try {
							mClientSocket.close();
							mClientSocket = null;
						} catch (Exception e) {
							e.printStackTrace();
						}
					}
				}
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				if (mServerSocket != null) {
					try {
						mServerSocket.close();
						mServerSocket = null;
					} catch (IOException e) {
						e.printStackTrace();
					}
				}
			}
		}
	}
}
