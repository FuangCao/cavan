package com.cavan.cavanmain;

import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.List;

import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.graphics.Color;
import android.inputmethodservice.InputMethodService;
import android.inputmethodservice.Keyboard;
import android.inputmethodservice.KeyboardView;
import android.inputmethodservice.KeyboardView.OnKeyboardActionListener;
import android.os.Handler;
import android.os.Message;
import android.view.Gravity;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.GridView;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;
import com.cavan.android.SystemProperties;
import com.cavan.java.CavanJava;
import com.cavan.java.CavanString;
import com.cavan.java.CavanTcpPacketClient;

public class CavanInputMethod extends InputMethodService implements OnKeyboardActionListener {

	private static final int CODE_MAX_COLUMNS = 3;
	private static final int AUTO_COMMIT_DELAY = 100;

	public static final int KEYCODE_SHIFT = -1;
    public static final int KEYCODE_MODE_CHANGE = -2;
    public static final int KEYCODE_CANCEL = -3;
    public static final int KEYCODE_DONE = -4;
    public static final int KEYCODE_DELETE = -5;
    public static final int KEYCODE_ALT = -6;

	public static final int KEYCODE_COPY = 1;
	public static final int KEYCODE_PASTE = 2;
	public static final int KEYCODE_CUT = 3;
	public static final int KEYCODE_SELECT = 4;
	public static final int KEYCODE_SELECT_ALL = 5;
	public static final int KEYCODE_LEFT = 6;
	public static final int KEYCODE_RIGHT = 7;
	public static final int KEYCODE_INPUT_METHOD = 8;
	public static final int KEYCODE_CLEAR = 10;
	public static final int KEYCODE_ENTER = 12;
	public static final int KEYCODE_SPACE = 13;
	public static final int KEYCODE_OCR = 14;
	public static final int KEYCODE_KEYBORD_NUMBER = 17;
	public static final int KEYCODE_KEYBORD_LETTER = 18;
	public static final int KEYCODE_KEYBORD_SYMBOL = 19;
	public static final int KEYCODE_KEYBORD_SELECT = 20;
	public static final int KEYCODE_CLIP_BOARD = 21;
	public static final int KEYCODE_ALIPAY = 22;
	public static final int KEYCODE_TAOBAO = 23;
	public static final int KEYCODE_TMALL = 24;
	public static final int KEYCODE_QQ = 25;
	public static final int KEYCODE_MM = 26;
	public static final int KEYCODE_MESSAGE = 27;
	public static final int KEYCODE_SERVICE = 28;
	public static final int KEYCODE_NAME = 29;
	public static final int KEYCODE_PHONE = 30;
	public static final int KEYCODE_HIDDEN = 31;
	public static final int KEYCODE_SPLIT = 32;
	public static final int KEYCODE_AUTO = 33;

	private static final int MSG_UPDATE_TCP_CLIENT = 1;
	private static final int MSG_TCP_CLIENT_UPDATED = 2;
	private static final int MSG_TCP_PACKET_RECEIVED = 3;
	private static final int MSG_REPLACE_TEXT = 4;
	private static final int MSG_INSERT_TEXT = 5;
	private static final int MSG_SEND_TEXT = 6;
	private static final int MSG_AUTO_SEND = 7;

	private static CavanInputMethod sInstance;

	public static CavanInputMethod getInstance() {
		return sInstance;
	}

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			removeMessages(msg.what);

			switch (msg.what) {
			case MSG_UPDATE_TCP_CLIENT:
				InetSocketAddress[] addresses = CavanMessageActivity.getNetworkImeAddresses(getApplicationContext());
				sTcpClient.setAddresses(addresses);
				if (addresses.length > 0) {
					sTcpClient.connect();
				}
				break;

			case MSG_TCP_CLIENT_UPDATED:
				CavanMessageActivity activity = CavanMessageActivity.getInstance();
				if (activity != null) {
					activity.updateNetworkImeState();
				}
				break;

			case MSG_TCP_PACKET_RECEIVED:
				onTcpPacketReceived((String) msg.obj);
				break;

			case MSG_REPLACE_TEXT:
				InputConnection conn = getCurrentInputConnection();
				if (conn != null && conn.performContextMenuAction(android.R.id.selectAll)) {
					CharSequence text = (CharSequence) msg.obj;
					if (text == null) {
						text = CavanString.EMPTY_STRING;
					}

					conn.commitText(text, 0);
				}
				break;

			case MSG_INSERT_TEXT:
				conn = getCurrentInputConnection();
				if (conn != null) {
					CharSequence text = (CharSequence) msg.obj;
					if (text != null) {
						conn.commitText(text, 0);
					}
				}
				break;

			case MSG_SEND_TEXT:
				CavanAccessibilityService accessibility = CavanAccessibilityService.getInstance();
				if (accessibility != null && accessibility.commitText(CavanInputMethod.this)) {
					if (mAutoSendText != null) {
						int delay = CavanMessageActivity.getRepeatDelay(CavanInputMethod.this);
						sendEmptyMessageDelayed(MSG_AUTO_SEND, delay);
					}
				}
				break;

			case MSG_AUTO_SEND:
				CharSequence text = mAutoSendText;
				if (text == null) {
					break;
				}

				conn = getCurrentInputConnection();
				if (conn == null) {
					break;
				}

				if (conn.performContextMenuAction(android.R.id.selectAll) && conn.commitText(text, 0)) {
					sendEmptyMessageDelayed(MSG_SEND_TEXT, AUTO_COMMIT_DELAY);
				}
				break;
			}
		}
	};

	private GridView mGridViewCodes;
	private RedPacketCode[] mUiCodes;
	private RedPacketViewAdapter mAdapterCodes = new RedPacketViewAdapter();

	private GridView mGridViewLines;
	private BaseAdapter mAdapterLines = new BaseAdapter() {

		private String[] mLines = new String[0];
		private OnClickListener mOnClickListener = new OnClickListener() {

			@Override
			public void onClick(View v) {
				Button button = (Button) v;
				InputConnection conn = getCurrentInputConnection();
				if (conn != null) {
					conn.commitText(button.getText(), 1);
				}
			}
		};

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			Button button;

			if (convertView != null) {
				button = (Button) convertView;
			} else {
				button = new Button(CavanInputMethod.this);
				button.setOnClickListener(mOnClickListener);
				button.setBackgroundColor(Color.BLACK);
				button.setTextColor(Color.WHITE);
			}

			button.setText(mLines[position].trim());
			button.setAllCaps(false);

			return button;
		}

		@Override
		public long getItemId(int position) {
			return position;
		}

		@Override
		public Object getItem(int position) {
			return mLines[position];
		}

		@Override
		public int getCount() {
			return mLines.length;
		}

		private String[] getClipboardLines() {
			String text = CavanAndroid.getClipboardText(getApplicationContext());
			if (text == null) {
				return null;
			}

			String[] lines = text.split("\\s*\\n\\s*");
			if (lines.length > 1 || lines[0].length() > 0) {
				return lines;
			}

			return null;
		}

		@Override
		public void notifyDataSetChanged() {
			String[] lines = getClipboardLines();
			if (lines != null) {
				int height;

				if (lines.length < 3) {
					height = LayoutParams.WRAP_CONTENT;
				} else {
					height = 500;
				}

				mGridViewLines.getLayoutParams().height = height;
				mLines = lines;

				super.notifyDataSetChanged();
			}
		}
	};

	private static NetworkImeTcpClient sTcpClient = new NetworkImeTcpClient();
	public static class NetworkImeTcpClient extends CavanTcpPacketClient {

		private CavanInputMethod mInputMethod;

		public synchronized void setInputMethod(CavanInputMethod ime) {
			mInputMethod = ime;

			if (ime == null) {
				disconnect();
			} else {
				ime.updateTcpClient();
			}
		}

		private synchronized void updateMessageActivity() {
			if (mInputMethod != null) {
				mInputMethod.getHandler().sendEmptyMessageDelayed(MSG_TCP_CLIENT_UPDATED, 500);
			}
		}

		@Override
		protected void onTcpClientStopped() {
			updateMessageActivity();
		}

		@Override
		protected void onTcpConnecting(InetSocketAddress address) {
			updateMessageActivity();
		}

		@Override
		protected boolean onTcpConnected(Socket socket) {
			String hostname = SystemProperties.get("net.hostname");
			if (hostname != null) {
				send("USER " + hostname);
			}

			updateMessageActivity();

			return true;
		}

		@Override
		protected boolean onTcpConnFailed(int times) {
			updateMessageActivity();
			return true;
		}

		@Override
		protected synchronized boolean onPacketReceived(byte[] bytes, int length) {
			if (mInputMethod != null) {
				Message message = mInputMethod.getHandler().obtainMessage(MSG_TCP_PACKET_RECEIVED, new String(bytes, 0, length));
				message.sendToTarget();
				return true;
			}

			return false;
		}
	};

	private Keyboard mKeyboardHead;
	private Keyboard mKeyboardNumber;
	private Keyboard mKeyboardSymbol;
	private Keyboard mKeyboardSelect;
	private Keyboard mKeyboardLetterLower;
	private Keyboard mKeyboardLetterUpper;
	private Keyboard mKeyboardLetterUpperLocked;
	private KeyboardView mKeyboardViewHead;
	private KeyboardView mKeyboardViewBody;
	private InputMethodManager mManager;

	private boolean mIsAlipay;
	private boolean mNeedPrefix;
	private boolean mAutoCommitEnable;
	private boolean mSelectionActive;
	private int mSelection;
	private int mSelectionStart;
	private int mSelectionEnd;

	private CharSequence mAutoSendText;
	private CharSequence mAutoSendTextPrev;

	public static boolean isDefaultInputMethod(Context context) {
		return "com.cavan.cavanmain/.CavanInputMethod".equals(CavanAndroid.getDefaultInputMethod(context));
	}

	public Handler getHandler() {
		return mHandler;
	}

	public boolean sendGoAction(InputConnection conn) {
		int action = EditorInfo.IME_ACTION_GO;
		EditorInfo info = getCurrentInputEditorInfo();

		if (info != null && info.actionLabel != null) {
			action = info.actionId;
		}

		return conn.performEditorAction(action);
	}

	public boolean sendGoAction() {
		InputConnection conn = getCurrentInputConnection();
		if (conn == null) {
			return false;
		}

		return sendGoAction(conn);
	}

	public boolean commitText(String text, boolean go) {
		InputConnection conn = getCurrentInputConnection();
		if (conn == null) {
			return false;
		}

		if (!conn.performContextMenuAction(android.R.id.selectAll)) {
			return false;
		}

		if (!conn.commitText(text, 1)) {
			return false;
		}

		if (go) {
			return sendGoAction(conn);
		}

		return true;
	}

	public boolean sendRedPacketCode(CharSequence code, boolean execute) {
		InputConnection conn = getCurrentInputConnection();
		if (conn == null) {
			return false;
		}

		if (mIsAlipay) {
			conn.performContextMenuAction(android.R.id.selectAll);
		} else if (mNeedPrefix) {
			code = "支付宝红包口令：" + code;
		}

		conn.commitText(code, 0);

		if (mIsAlipay && execute) {
			sendGoAction(conn);
		}

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

	private void setKeyboard(Keyboard keyboard) {
		if (keyboard != null) {
			mGridViewLines.setVisibility(View.GONE);
			mKeyboardViewBody.setKeyboard(keyboard);
			mKeyboardViewBody.setVisibility(View.VISIBLE);
		} else {
			mKeyboardViewBody.setVisibility(View.GONE);
			mAdapterLines.notifyDataSetChanged();
			mGridViewLines.setVisibility(View.VISIBLE);
		}
	}

	private void setAutoSendText(CharSequence text) {
		CavanAndroid.dLog("setAutoSendText: " + text);

		if (text == null) {
			text = mAutoSendText;
			mAutoSendText = null;

			mHandler.removeMessages(MSG_AUTO_SEND);

			if (text != null) {
				Message message = mHandler.obtainMessage(MSG_REPLACE_TEXT, null);
				mHandler.sendMessageDelayed(message, 200);
				mAutoSendTextPrev = text;
			}
		} else {
			mAutoSendText = text;
			mHandler.sendEmptyMessageDelayed(MSG_SEND_TEXT, AUTO_COMMIT_DELAY);
		}
	}

	@Override
	public void onCreate() {
		super.onCreate();

		mManager = (InputMethodManager) getSystemService(INPUT_METHOD_SERVICE);
		sTcpClient.setInputMethod(this);
		sInstance = this;
	}

	@Override
	public void onDestroy() {
		sTcpClient.setInputMethod(null);
		sInstance = null;
		super.onDestroy();
	}

	@Override
	public void onUpdateSelection(int oldSelStart, int oldSelEnd, int newSelStart, int newSelEnd, int candidatesStart, int candidatesEnd) {
		if (newSelStart < newSelEnd) {
			mSelectionActive = true;
		} else {
			mSelection = newSelStart;

			if (mAutoCommitEnable && newSelEnd == 8) {
				InputConnection conn = getCurrentInputConnection();
				if (conn != null) {
					CharSequence text = conn.getTextBeforeCursor(8, 0);
					if (text != null && CavanJava.isDigit(text)) {
						text = conn.getTextAfterCursor(1, 0);
						if (text == null || text.length() <= 0) {
							sendGoAction(conn);
						}
					}
				}
			}
		}

		mSelectionStart = newSelStart;
		mSelectionEnd = newSelEnd;

		super.onUpdateSelection(oldSelStart, oldSelEnd, newSelStart, newSelEnd, candidatesStart, candidatesEnd);
	}

	@Override
	public void onStartInput(EditorInfo attribute, boolean restarting) {
		String pkgName = getCurrentInputEditorInfo().packageName;
		CavanAndroid.dLog("onStartInput: package = " + pkgName);

		mIsAlipay = CavanPackageName.ALIPAY.equals(pkgName);
		if (mIsAlipay) {
			mNeedPrefix = false;
		} else if (CavanPackageName.CALENDAR.equals(pkgName)) {
			mNeedPrefix = true;
		} else {
			mNeedPrefix = false;
		}

		mAutoCommitEnable = false;
		super.onStartInput(attribute, restarting);
	}

	@Override
	public void onStartInputView(EditorInfo info, boolean restarting) {
		mAdapterCodes.updateInputView();
		// setKeyboard(mKeyboardNumber);
		super.onStartInputView(info, restarting);
	}

	@Override
	public void onFinishInputView(boolean finishingInput) {
		setAutoSendText(null);
		mAutoCommitEnable = false;
		super.onFinishInputView(finishingInput);
	}

	@Override
	public View onCreateInputView() {
		mKeyboardHead = new Keyboard(this, R.xml.keyboard_head);
		mKeyboardNumber = new Keyboard(this, R.xml.keyboard_number);
		mKeyboardSymbol = new Keyboard(this, R.xml.keyboard_symbol);
		mKeyboardSelect = new Keyboard(this, R.xml.keyboard_select);
		mKeyboardLetterLower = new Keyboard(this, R.xml.keyboard_lowercase);
		mKeyboardLetterUpper = new Keyboard(this, R.xml.keyboard_uppercase);
		mKeyboardLetterUpperLocked = new Keyboard(this, R.xml.keyboard_uppercase_locked);

		mKeyboardLetterUpper.setShifted(true);
		mKeyboardLetterUpperLocked.setShifted(true);

		View view = View.inflate(this, R.layout.keyboard, null);

		mGridViewCodes = (GridView) view.findViewById(R.id.gridViewCodes);
		mGridViewCodes.setAdapter(mAdapterCodes);
		mAdapterCodes.updateInputView();

		mGridViewLines = (GridView) view.findViewById(R.id.gridViewLines);
		mGridViewLines.setAdapter(mAdapterLines);

		mKeyboardViewHead = (KeyboardView) view.findViewById(R.id.keyboardViewHead);
		mKeyboardViewHead.setKeyboard(mKeyboardHead);
		mKeyboardViewHead.setEnabled(true);
		mKeyboardViewHead.setPreviewEnabled(false);
		mKeyboardViewHead.setOnKeyboardActionListener(this);

		mKeyboardViewBody = (KeyboardView) view.findViewById(R.id.keyboardViewBody);
		setKeyboard(mKeyboardNumber);
		mKeyboardViewBody.setEnabled(true);
		mKeyboardViewBody.setPreviewEnabled(false);
		mKeyboardViewBody.setOnKeyboardActionListener(this);

		return view;
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

			mSelectionActive = false;
			break;

		case KEYCODE_CLEAR:
			conn.performContextMenuAction(android.R.id.selectAll);
			conn.commitText(CavanString.EMPTY_STRING, 0);
			break;

		case KEYCODE_DONE:
			sendGoAction(conn);
			break;

		case KEYCODE_COPY:
			conn.performContextMenuAction(android.R.id.copy);
			break;

		case KEYCODE_CUT:
			mSelectionActive = false;
			conn.performContextMenuAction(android.R.id.cut);
			break;

		case KEYCODE_PASTE:
			mSelectionActive = false;
			conn.performContextMenuAction(android.R.id.paste);
			break;

		case KEYCODE_SELECT_ALL:
			mSelectionActive = true;
			conn.performContextMenuAction(android.R.id.selectAll);
			break;

		case KEYCODE_INPUT_METHOD:
			mManager.showInputMethodPicker();
			break;

		case KEYCODE_SELECT:
			if (mSelectionActive) {
				mSelectionActive = false;
				conn.setSelection(mSelection, mSelection);
			} else {
				mSelectionActive = true;
			}
			break;

		case KEYCODE_LEFT:
			if (mSelectionActive) {
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
			if (mSelectionActive) {
				if (mSelection > mSelectionStart) {
					conn.setSelection(mSelectionStart, ++mSelection);
				} else {
					conn.setSelection(++mSelection, mSelectionEnd);
				}
			} else {
				conn.setSelection(mSelection + 1, mSelection + 1);
			}
			break;

		case KEYCODE_OCR:
			CavanMessageActivity.startSogouOcrActivity(getApplicationContext());
			break;

		case KEYCODE_SHIFT:
			Keyboard keyboard = mKeyboardViewBody.getKeyboard();
			if (keyboard == mKeyboardLetterLower) {
				setKeyboard(mKeyboardLetterUpper);
				break;
			}

			if (keyboard == mKeyboardLetterUpper) {
				setKeyboard(mKeyboardLetterUpperLocked);
				break;
			}
		case KEYCODE_KEYBORD_LETTER:
			setKeyboard(mKeyboardLetterLower);
			break;

		case KEYCODE_KEYBORD_NUMBER:
			setKeyboard(mKeyboardNumber);
			break;

		case KEYCODE_KEYBORD_SYMBOL:
			setKeyboard(mKeyboardSymbol);
			break;

		case KEYCODE_KEYBORD_SELECT:
			setKeyboard(mKeyboardSelect);
			break;

		case KEYCODE_ALIPAY:
			CavanAndroid.startActivity(this, CavanPackageName.ALIPAY);
			break;

		case KEYCODE_TAOBAO:
			CavanAndroid.startActivity(this, CavanPackageName.TAOBAO);
			break;

		case KEYCODE_TMALL:
			CavanAndroid.startActivity(this, CavanPackageName.TMALL);
			break;

		case KEYCODE_QQ:
			CavanAndroid.startActivity(this, CavanPackageName.QQ);
			break;

		case KEYCODE_MM:
			CavanAndroid.startActivity(this, CavanPackageName.MM);
			break;

		case KEYCODE_CLIP_BOARD:
			ClipboardManager manager = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);
			ClipData clip = manager.getPrimaryClip();

			if (clip != null && clip.getItemCount() > 0) {
				text = clip.getItemAt(0).coerceToText(this);
				if (text != null) {
					FloatEditorDialog dialog = FloatEditorDialog.getInstance(this, text, false, false);
					dialog.show(20000);
				}
			}
			break;

		case KEYCODE_MESSAGE:
			CavanAndroid.startActivity(this, CavanMessageActivity.class);
			break;

		case KEYCODE_SERVICE:
			CavanAndroid.startActivity(this, CavanServiceActivity.class);
			break;

		case KEYCODE_NAME:
			text = CavanMessageActivity.getName(this);
			if (text != null) {
				conn.commitText(text, 1);
			}
			break;

		case KEYCODE_PHONE:
			text = CavanMessageActivity.getPhone(this);
			if (text != null) {
				conn.commitText(text, 1);
			}
			break;

		case KEYCODE_HIDDEN:
			hideWindow();
			break;

		case KEYCODE_SPLIT:
			setKeyboard(null);
			break;

		case KEYCODE_AUTO:
			if (mAutoSendText != null) {
				setAutoSendText(null);
			} else if (conn.performContextMenuAction(android.R.id.selectAll)) {
				text = conn.getSelectedText(0);
				if (text == null || text.length() <= 0) {
					text = mAutoSendTextPrev;
					if (text == null) {
						text = CavanAndroid.getClipboardText(CavanInputMethod.this);
						if (text != null && text.length() <= 0) {
							text = null;
						}
					}

					if (text != null) {
						conn.commitText(text, 0);
					}
				}

				setAutoSendText(text);
			}
			break;
		}
	}

	@Override
	public void onText(CharSequence text) {
		mSelectionActive = false;

		if (mIsAlipay) {
			mAutoCommitEnable = true;
		}

		InputConnection conn = getCurrentInputConnection();
		if (conn != null) {
			conn.commitText(text, 1);
		}

		if (mKeyboardViewBody.getKeyboard() == mKeyboardLetterUpper) {
			setKeyboard(mKeyboardLetterLower);
		}
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

	public class RedPacketView extends Button implements OnClickListener {

		private RedPacketCode mCode;

		public RedPacketView(Context context) {
			super(context);

			setSingleLine();
			setGravity(Gravity.CENTER);
			setBackgroundColor(Color.WHITE);
			setOnClickListener(this);
		}

		public void setRedPacketCode(RedPacketCode code) {
			mCode = code;
			setText(code.getCode());
			setTextColor(code.isInvalid() ? Color.RED : Color.BLACK);
		}

		public RedPacketCode getPacketCode() {
			return mCode;
		}

		@Override
		public void onClick(View v) {
			sendRedPacketCode(mCode.getCode(), !mCode.isInvalid());
		}
	}

	public class RedPacketViewAdapter extends BaseAdapter {

		private Handler mHandler = new Handler() {

			@Override
			public void handleMessage(Message msg) {
				mHandler.removeMessages(0);

				List<RedPacketCode> codes = RedPacketCode.getLastCodes();
				if (codes == null) {
					return;
				}

				int lines;
				int columns;
				int size = codes.size();

				if (size > CODE_MAX_COLUMNS) {
					lines = (size + CODE_MAX_COLUMNS - 1) / CODE_MAX_COLUMNS;
					columns = (size + lines - 1) / lines;

					if (lines > 2) {
						lines = 2;
					}
				} else {
					if (size > 0) {
						lines = 1;
					} else {
						lines = 0;
					}

					columns = size;
				}

				int height;

				if (lines > 0) {
					View view = mGridViewCodes.getChildAt(0);
					if (view != null) {
						height = view.getHeight() * lines;
					} else {
						height = LayoutParams.WRAP_CONTENT;
					}

					mGridViewCodes.setNumColumns(columns);
				} else {
					height = LayoutParams.WRAP_CONTENT;
				}

				mGridViewCodes.getLayoutParams().height = height;

				mUiCodes = new RedPacketCode[size];
				codes.toArray(mUiCodes);

				notifyDataSetChanged();
			}
		};

		public void updateInputView() {
			mHandler.sendEmptyMessageDelayed(0, 200);
		}

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			RedPacketView view;

			if (convertView != null) {
				view = (RedPacketView) convertView;
			} else {
				view = new RedPacketView(getApplicationContext());
			}

			view.setRedPacketCode(mUiCodes[position]);

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
			if (mUiCodes != null) {
				return mUiCodes.length;
			}

			return 0;
		}
	}

	protected void onTcpPacketReceived(String command) {
		CavanAndroid.dLog("onTcpPacketReceived: " + command);

		String[] args = command.split(" ", 2);
		InputConnection conn = getCurrentInputConnection();
		if (conn == null) {
			return;
		}

		boolean send = false;

		switch (args[0]) {
		case "SEND":
			send = true;
		case "REPLACE":
			if (!conn.performContextMenuAction(android.R.id.selectAll)) {
				break;
			}
		case "INSERT":
			if (args.length > 1) {
				conn.commitText(args[1], 0);

				if (send) {
					mHandler.sendEmptyMessageDelayed(MSG_SEND_TEXT, AUTO_COMMIT_DELAY);
				}
			} else {
				conn.commitText(CavanString.EMPTY_STRING, 0);
			}
			break;

		case "DELETE":
			CharSequence text = conn.getSelectedText(0);
			if (text != null && text.length() > 0) {
				conn.commitText(CavanString.EMPTY_STRING, 1);
			} else {
				conn.deleteSurroundingText(1, 0);
			}
			break;

		case "COMMIT":
			CavanAccessibilityService accessibility = CavanAccessibilityService.getInstance();
			if (accessibility != null && accessibility.commitText(CavanInputMethod.this)) {
				break;
			}
		case "DONE":
			sendGoAction(conn);
			break;

		case "KEY":
			if (args.length > 1) {
				try {
					args = args[1].split("\\s+");

					int code = Integer.parseInt(args[0].trim());

					if (args.length > 1) {
						int value = Integer.parseInt(args[1].trim());
						sendKeyEvent(code, value);
					} else {
						sendKeyDownUp(code);
					}
				} catch (NumberFormatException e) {
					e.printStackTrace();
				}
			}
			break;

		case "ACTION":
			if (args.length > 0) {
				try {
					int action = Integer.parseInt(args[1].trim());
					conn.performEditorAction(action);
				} catch (NumberFormatException e) {
					e.printStackTrace();
				}
			}
			break;

		case "MENU_ACTION":
			if (args.length > 0) {
				try {
					int action = Integer.parseInt(args[1].trim());
					conn.performContextMenuAction(action);
				} catch (NumberFormatException e) {
					e.printStackTrace();
				}
			}
			break;

		case "OPEN":
			if (args.length > 0) {
				CavanAndroid.startActivity(CavanInputMethod.this, args[1]);
			}
			break;

		case "CLIPBOARD":
			if (args.length > 0) {
				CavanAndroid.postClipboardText(CavanInputMethod.this, args[1]);
			}
			break;

		case "COPY":
			conn.performContextMenuAction(android.R.id.copy);
			break;

		case "PASTE":
			conn.performContextMenuAction(android.R.id.paste);
			break;

		case "SELECT_ALL":
			conn.performContextMenuAction(android.R.id.selectAll);
			break;
		}
	}

	public CavanTcpPacketClient getTcpClient() {
		return sTcpClient;
	}

	public void updateTcpClient() {
		mHandler.sendEmptyMessageDelayed(MSG_UPDATE_TCP_CLIENT, 200);
	}
}
