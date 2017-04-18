package com.cavan.android;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Process;

public class CavanThreadedHandler extends Handler {

	public CavanThreadedHandler(Callback callback, Class<?> cls, int index, int priority) {
		super(CavanHandlerThread.getLooper(cls, index, priority), callback);
	}

	public CavanThreadedHandler(Class<?> cls, int index, int priority) {
		this(null, cls, index, priority);
	}

	public CavanThreadedHandler(Callback callback, Class<?> cls, int index) {
		this(callback, cls, index, Process.THREAD_PRIORITY_DEFAULT);
	}

	public CavanThreadedHandler(Class<?> cls, int index) {
		this(null, cls, index);
	}

	public CavanThreadedHandler(Class<?> cls) {
		this(cls, 0);
	}

	public CavanThreadedHandler(Callback callback, Class<?> cls) {
		this(callback, cls, 0);
	}

	public boolean quit() {
		Looper looper = getLooper();
		if (looper == null) {
			return false;
		}

		looper.quit();

		return true;
	}

	public boolean quitSafely() {
		Looper looper = getLooper();
		if (looper == null) {
			return false;
		}

		looper.quitSafely();

		return true;
	}

	public Message obtainMessage(int what, int arg1) {
		Message message = obtainMessage(what);
		message.arg1 = arg1;
		return message;
	}

	public Message obtainMessage(int what, Object obj, int arg1) {
		Message message = obtainMessage(what, obj);
		message.arg1 = arg1;
		return message;
	}

	public Message obtainMessage(int what, Object obj, int arg1, int arg2) {
		Message message = obtainMessage(what, obj, arg1);
		message.arg2 = arg2;
		return message;
	}

	public Message obtainMessageWithArgs(int what, Object... args) {
		return obtainMessage(what, args);
	}

	public void sendMessage(int what, int arg1) {
		obtainMessage(what, arg1).sendToTarget();
	}

	public void sendMessage(int what, int arg1, int arg2) {
		obtainMessage(what, arg1, arg2).sendToTarget();
	}

	public void sendMessage(int what, Object obj) {
		obtainMessage(what, obj).sendToTarget();
	}

	public void sendMessage(int what, Object obj, int arg1) {
		obtainMessage(what, obj, arg1).sendToTarget();
	}

	public void sendMessage(int what, Object obj, int arg1, int arg2) {
		obtainMessage(what, obj, arg1, arg2).sendToTarget();
	}

	public void sendMessageWithArgs(int what, Object... args) {
		obtainMessage(what, args).sendToTarget();
	}
}
