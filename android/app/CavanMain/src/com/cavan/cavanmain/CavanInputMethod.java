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
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ListView;

import com.cavan.java.CavanString;

public class CavanInputMethod extends InputMethodService implements OnClickListener, OnKeyboardActionListener {

	public static final int KEYCODE_COPY = 1;
	public static final int KEYCODE_PASTE = 2;
	public static final int KEYCODE_CUT = 3;
	public static final int KEYCODE_SELECT = 4;
	public static final int KEYCODE_SELECT_ALL = 5;
	public static final int KEYCODE_LEFT = 6;
	public static final int KEYCODE_RIGHT = 7;
	public static final int KEYCODE_INPUT_METHOD = 8;

	private List<String> mCodes;
	private ListView mListViewCodes;

	private Keyboard mKeyboard;
	private KeyboardView mKeyboardView;

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
		if (mService != null) {
			try {
				mCodes = mService.getCodes();
				mAdapter.notifyDataSetChanged();
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}
	}

	public void performEditorAction(InputConnection conn) {
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
		performEditorAction(conn);
	}

	@Override
	public void onCreate() {
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
		}

		mSelectionStart = newSelStart;
		mSelectionEnd = newSelEnd;

		super.onUpdateSelection(oldSelStart, oldSelEnd, newSelStart, newSelEnd,
				candidatesStart, candidatesEnd);
	}

	@Override
	public void onStartInput(EditorInfo attribute, boolean restarting) {
		updateData();
		super.onStartInput(attribute, restarting);
	}

	@Override
	public void onViewClicked(boolean focusChanged) {
		if (mCodes != null && mCodes.size() == 1) {
			sendRedPacketCode(mCodes.get(0));
		}

		super.onViewClicked(focusChanged);
	}

	@Override
	public View onCreateInputView() {
		View view = View.inflate(this, R.layout.keyboard, null);

		mListViewCodes = (ListView) view.findViewById(R.id.listViewCodes);
		mListViewCodes.setAdapter(mAdapter);

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
		case Keyboard.KEYCODE_DELETE:
			CharSequence text = conn.getSelectedText(0);
			if (text != null && text.length() > 0) {
				conn.commitText(CavanString.EMPTY_STRING, 1);
			} else {
				conn.deleteSurroundingText(1, 0);
			}

			mSelectioActive = false;
			break;

		case Keyboard.KEYCODE_DONE:
			performEditorAction(conn);
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
			conn.performContextMenuAction(android.R.id.switchInputMethod);
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
