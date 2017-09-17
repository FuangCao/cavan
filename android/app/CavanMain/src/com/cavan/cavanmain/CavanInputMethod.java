package com.cavan.cavanmain;

import java.util.List;

import android.content.Context;
import android.graphics.Color;
import android.inputmethodservice.InputMethodService;
import android.inputmethodservice.Keyboard;
import android.inputmethodservice.KeyboardView;
import android.inputmethodservice.KeyboardView.OnKeyboardActionListener;
import android.os.Handler;
import android.os.Message;
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
import com.cavan.java.CavanJava;
import com.cavan.java.CavanString;

public class CavanInputMethod extends InputMethodService implements OnKeyboardActionListener {

	private static final int CODE_MAX_COLUMNS = 3;

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
	public static final int KEYCODE_OCR = 14;
	public static final int KEYCODE_UPPERCASE = 15;
	public static final int KEYCODE_LOWERCASE = 16;
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

	private static CavanInputMethod sInstance;

	public static CavanInputMethod getInstance() {
		return sInstance;
	}

	private GridView mGridViewCodes;
	private RedPacketCode[] mUiCodes;
	private RedPacketViewAdapter mAdapter = new RedPacketViewAdapter();

	private Keyboard mKeyboardHead;
	private Keyboard mKeyboardNumber;
	private Keyboard mKeyboardSymbol;
	private Keyboard mKeyboardSelect;
	private Keyboard mKeyboardLetterLower;
	private Keyboard mKeyboardLetterUpper;
	private KeyboardView mKeyboardViewHead;
	private KeyboardView mKeyboardViewBody;
	private InputMethodManager mManager;

	private boolean mIsAlipay;
	private boolean mNeedPrefix;
	private boolean mAutoCommitEnable;
	private boolean mSelectioActive;
	private int mSelection;
	private int mSelectionStart;
	private int mSelectionEnd;

	public static boolean isDefaultInputMethod(Context context) {
		return "com.cavan.cavanmain/.CavanInputMethod".equals(CavanAndroid.getDefaultInputMethod(context));
	}

	public boolean sendFinishAction(InputConnection conn) {
		int action = EditorInfo.IME_ACTION_GO;
		EditorInfo info = getCurrentInputEditorInfo();

		if (info != null && info.actionLabel != null) {
			action = info.actionId;
		}

		return conn.performEditorAction(action);
	}

	public boolean sendFinishAction() {
		InputConnection conn = getCurrentInputConnection();
		if (conn == null) {
			return false;
		}

		return sendFinishAction(conn);
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
			sendFinishAction(conn);
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

	@Override
	public void onCreate() {
		super.onCreate();

		mManager = (InputMethodManager) getSystemService(INPUT_METHOD_SERVICE);
		sInstance = this;
	}

	@Override
	public void onDestroy() {
		sInstance = null;
		super.onDestroy();
	}

	@Override
	public void onUpdateSelection(int oldSelStart, int oldSelEnd, int newSelStart, int newSelEnd, int candidatesStart, int candidatesEnd) {
		if (newSelStart < newSelEnd) {
			mSelectioActive = true;
		} else {
			mSelection = newSelStart;

			if (mAutoCommitEnable && newSelEnd == 8) {
				InputConnection conn = getCurrentInputConnection();
				if (conn != null) {
					CharSequence text = conn.getTextBeforeCursor(8, 0);
					if (text != null && CavanJava.isDigit(text)) {
						text = conn.getTextAfterCursor(1, 0);
						if (text == null || text.length() <= 0) {
							sendFinishAction(conn);
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
		mAdapter.updateInputView();
		// mKeyboardViewBody.setKeyboard(mKeyboardNumber);
		super.onStartInputView(info, restarting);
	}

	@Override
	public void onFinishInputView(boolean finishingInput) {
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

		View view = View.inflate(this, R.layout.keyboard, null);

		mGridViewCodes = (GridView) view.findViewById(R.id.gridViewCodes);
		mGridViewCodes.setAdapter(mAdapter);
		mAdapter.updateInputView();

		mKeyboardViewHead = (KeyboardView) view.findViewById(R.id.keyboardViewHead);
		mKeyboardViewHead.setKeyboard(mKeyboardHead);
		mKeyboardViewHead.setEnabled(true);
		mKeyboardViewHead.setPreviewEnabled(false);
		mKeyboardViewHead.setOnKeyboardActionListener(this);

		mKeyboardViewBody = (KeyboardView) view.findViewById(R.id.keyboardViewBody);
		mKeyboardViewBody.setKeyboard(mKeyboardNumber);
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

		case KEYCODE_OCR:
			CavanMessageActivity.startSogouOcrActivity(getApplicationContext());
			break;

		case KEYCODE_UPPERCASE:
			mKeyboardViewBody.setKeyboard(mKeyboardLetterUpper);
			break;

		case KEYCODE_LOWERCASE:
		case KEYCODE_KEYBORD_LETTER:
			mKeyboardViewBody.setKeyboard(mKeyboardLetterLower);
			break;

		case KEYCODE_KEYBORD_NUMBER:
			mKeyboardViewBody.setKeyboard(mKeyboardNumber);
			break;

		case KEYCODE_KEYBORD_SYMBOL:
			mKeyboardViewBody.setKeyboard(mKeyboardSymbol);
			break;

		case KEYCODE_KEYBORD_SELECT:
			mKeyboardViewBody.setKeyboard(mKeyboardSelect);
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
		}
	}

	@Override
	public void onText(CharSequence text) {
		mSelectioActive = false;

		if (mIsAlipay) {
			mAutoCommitEnable = true;
		}

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

	public class RedPacketView extends Button implements OnClickListener {

		private RedPacketCode mCode;

		public RedPacketView(Context context) {
			super(context);
			setSingleLine();
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
				view = new RedPacketView(CavanInputMethod.this);
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
	};
}
