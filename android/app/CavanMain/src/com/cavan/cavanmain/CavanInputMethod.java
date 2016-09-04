package com.cavan.cavanmain;

import java.util.List;

import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.inputmethodservice.InputMethodService;
import android.inputmethodservice.Keyboard;
import android.inputmethodservice.KeyboardView;
import android.inputmethodservice.KeyboardView.OnKeyboardActionListener;
import android.os.IBinder;
import android.os.RemoteException;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.GridView;

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

	private List<String> mCodes;
	private GridView mCodeGridView;

	private Keyboard mKeyboard;
	private KeyboardView mKeyboardView;
	private InputMethodManager mManager;

	private boolean mIsAlipay;
	private boolean mSelectioActive;
	private int mSelection;
	private int mSelectionStart;
	private int mSelectionEnd;

	private BaseAdapter mAdapter = new BaseAdapter() {

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			Button view = new Button(CavanInputMethod.this);
			view.setOnClickListener(CavanInputMethod.this);
			view.setText(mCodes.get(position));
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
		if (mService != null && mCodeGridView != null) {
			try {
				mCodes = mService.getCodes();
				if (mCodes != null) {
					int columns = mCodes.size();
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

					mCodeGridView.setNumColumns(columns);
				}

				mAdapter.notifyDataSetChanged();
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}
	}

	public void sendFinishAction(InputConnection conn) {
		int action;
		EditorInfo info = getCurrentInputEditorInfo();

		if (info.actionLabel == null) {
			action = EditorInfo.IME_ACTION_GO;
		} else {
			action = info.actionId;
		}

		conn.performEditorAction(action);
	}

	public void sendRedPacketCode(CharSequence text) {
		InputConnection conn = getCurrentInputConnection();
		conn.performContextMenuAction(android.R.id.selectAll);
		conn.commitText(text, 0);
		sendFinishAction(conn);
	}

	@Override
	public void onCreate() {
		mManager = (InputMethodManager) getSystemService(INPUT_METHOD_SERVICE);

		Intent service = FloatMessageService.startService(this);
		bindService(service, mConnection, 0);

		super.onCreate();
	}

	@Override
	public void onDestroy() {
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
				CharSequence text = getCurrentInputConnection().getTextBeforeCursor(8, 0);
				if (text != null && text.length() == 8 && CavanJava.isDigit(text)) {
					sendFinishAction(getCurrentInputConnection());
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

		updateData();

		if (mIsAlipay && mCodes != null && mCodes.size() == 1) {
			sendRedPacketCode(mCodes.get(0));
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
}
