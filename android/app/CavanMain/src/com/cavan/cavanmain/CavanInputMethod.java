package com.cavan.cavanmain;

import java.util.List;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Color;
import android.inputmethodservice.InputMethodService;
import android.inputmethodservice.Keyboard;
import android.inputmethodservice.KeyboardView;
import android.inputmethodservice.KeyboardView.OnKeyboardActionListener;
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
import com.cavan.android.CavanPackageName;
import com.cavan.java.CavanJava;
import com.cavan.java.CavanString;

public class CavanInputMethod extends InputMethodService implements OnClickListener, OnKeyboardActionListener {

	private static final int CODE_MAX_COLUMNS = 4;

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

	private GridView mCodeGridView;
	private RedPacketCode[] mUiCodes;

	private Keyboard mKeyboard;
	private KeyboardView mKeyboardView;
	private InputMethodManager mManager;

	private boolean mIsAlipay;
	private boolean mAutoCommitEnable;
	private boolean mSelectioActive;
	private int mSelection;
	private int mSelectionStart;
	private int mSelectionEnd;

	private BroadcastReceiver mReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();

			CavanAndroid.dLog("action = " + action);

			if (action.equals(MainActivity.ACTION_CODE_COMMIT)) {
				InputConnection conn = getCurrentInputConnection();
				if (conn != null) {
					sendFinishAction(conn);
				}
			}
		}
	};

	private Runnable mRunnableUpdateInputView = new Runnable() {

		@Override
		public void run() {
			mCodeGridView.removeCallbacks(this);

			List<RedPacketCode> codes = RedPacketCode.getLastCodes();
			if (codes == null) {
				return;
			}

			int columns;
			int size = codes.size();

			if (size > CODE_MAX_COLUMNS) {
				int lines = (size + CODE_MAX_COLUMNS - 1) / CODE_MAX_COLUMNS;
				columns = (size + lines - 1) / lines;
			} else {
				columns = size;
			}

			mUiCodes = new RedPacketCode[size];
			codes.toArray(mUiCodes);

			mCodeGridView.setNumColumns(columns);
			mAdapter.notifyDataSetChanged();
		}
	};

	private BaseAdapter mAdapter = new BaseAdapter() {

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			Button view;
			RedPacketCode code = mUiCodes[position];

			if (convertView != null) {
				view = (Button) convertView;
			} else {
				view = new Button(CavanInputMethod.this);
			}

			view.setTextColor(code.isInvalid() ? Color.RED : Color.BLACK);
			view.setOnClickListener(CavanInputMethod.this);
			view.setText(code.getCode());
			view.setSingleLine();

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

	public static boolean isDefaultInputMethod(Context context) {
		return "com.cavan.cavanmain/.CavanInputMethod".equals(CavanAndroid.getDefaultInputMethod(context));
	}

	public void updateInputView() {
		if (mCodeGridView != null) {
			mCodeGridView.removeCallbacks(mRunnableUpdateInputView);
			mCodeGridView.postDelayed(mRunnableUpdateInputView, 100);
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

		IntentFilter filter = new IntentFilter();
		filter.addAction(MainActivity.ACTION_CODE_COMMIT);
		registerReceiver(mReceiver, filter);

		super.onCreate();
	}

	@Override
	public void onDestroy() {
		unregisterReceiver(mReceiver);
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
		mAutoCommitEnable = false;
		mIsAlipay = CavanPackageName.ALIPAY.equals(getCurrentInputEditorInfo().packageName);
		updateInputView();

		super.onStartInput(attribute, restarting);
	}

	@Override
	public void onFinishInputView(boolean finishingInput) {
		mAutoCommitEnable = false;
		super.onFinishInputView(finishingInput);
	}

	@Override
	public View onCreateInputView() {
		View view = View.inflate(this, R.layout.keyboard, null);

		mCodeGridView = (GridView) view.findViewById(R.id.gridViewCodes);
		mCodeGridView.setAdapter(mAdapter);
		updateInputView();

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
		CharSequence text = button.getText();

		if (!mIsAlipay) {
			text = "支付宝红包口令：" + text;
		}

		sendRedPacketCode(text);
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
			MainActivity.startSogouOcrActivity(getApplicationContext());
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
}
