package com.cavan.accessibility;

import android.inputmethodservice.InputMethodService;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;

public class CavanInputMethodService extends InputMethodService {

	public static CavanInputMethodService instance;

	public boolean performEditorAction(int action) {
		InputConnection conn = getCurrentInputConnection();
		if (conn == null) {
			return false;
		}

		return conn.performEditorAction(action);
	}

	public boolean performContextMenuAction(int id) {
		InputConnection conn = getCurrentInputConnection();
		if (conn == null) {
			return false;
		}

		return conn.performContextMenuAction(id);
	}

	public boolean commitText(InputConnection conn, CharSequence text, boolean replace) {
		if (replace && !conn.performContextMenuAction(android.R.id.selectAll)) {
			return false;
		}

		return conn.commitText(text, 0);
	}

	public boolean commitText(CharSequence text, boolean replace) {
		InputConnection conn = getCurrentInputConnection();
		if (conn == null) {
			return false;
		}

		return commitText(conn, text, replace);
	}

	public boolean sendKeyEvent(InputConnection conn, int code, int action) {
		long time = System.currentTimeMillis();
		KeyEvent event = new KeyEvent(time, time, action, code, 0, 0, KeyCharacterMap.VIRTUAL_KEYBOARD, 0, 0);
		return conn.sendKeyEvent(event);
	}

	public boolean sendKeyDown(InputConnection conn, int code) {
		return sendKeyEvent(conn, code, KeyEvent.ACTION_DOWN);
	}

	public boolean sendKeyDown(int code) {
		InputConnection conn = getCurrentInputConnection();
		if (conn == null) {
			return false;
		}

		return sendKeyDown(conn, code);
	}

	public boolean sendKeyUp(InputConnection conn, int code) {
		return sendKeyEvent(conn, code, KeyEvent.ACTION_UP);
	}

	public boolean sendKeyUp(int code) {
		InputConnection conn = getCurrentInputConnection();
		if (conn == null) {
			return false;
		}

		return sendKeyUp(conn, code);
	}

	public boolean sendKeyDownUp(InputConnection conn, int code) {
		return sendKeyDown(conn, code) && sendKeyUp(conn, code);
	}

	public boolean sendKeyDownUp(int code) {
		InputConnection conn = getCurrentInputConnection();
		if (conn == null) {
			return false;
		}

		return sendKeyDownUp(conn, code);
	}

	public boolean commitAlipayCode() {
		return performEditorAction(EditorInfo.IME_ACTION_GO);
	}

	public boolean inputAlipayCode(CharSequence code) {
		return commitText(code, true);
	}

	@Override
	public void onCreate() {
		super.onCreate();
		instance = this;
	}

	@Override
	public void onDestroy() {
		instance = null;
		super.onDestroy();
	}
}
