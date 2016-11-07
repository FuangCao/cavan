package com.cavan.cavanmain;

import com.cavan.android.CavanAndroid;

import android.content.Context;
import android.content.Intent;
import android.graphics.PixelFormat;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnKeyListener;
import android.view.View.OnTouchListener;
import android.view.WindowManager;
import android.view.WindowManager.LayoutParams;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;

public class FloatEditorDialog implements OnClickListener, Runnable, OnKeyListener, OnTouchListener {

	private Context mContext;
	private boolean mShowing;
	private boolean mAutoDismiss;

	private View mRootView;
	private Button mButtonCopy;
	private Button mButtonSend;
	private Button mButtonSecretOrder;

	private CheckBox mCheckBox;
	private EditText mEditText;
	private WindowManager mWindowManager;

	public FloatEditorDialog(Context context, CharSequence text, boolean checked) {
		mContext = context;

		mRootView = View.inflate(context, R.layout.float_editor, null);
		mRootView.setOnTouchListener(this);

		mButtonSend = (Button) findViewById(R.id.buttonSend);
		mButtonSend.setOnClickListener(this);

		mButtonCopy = (Button) findViewById(R.id.buttonCopy);
		mButtonCopy.setOnClickListener(this);

		mButtonSecretOrder = (Button) findViewById(R.id.buttonSecretOrder);
		mButtonSecretOrder.setOnClickListener(this);

		mCheckBox = (CheckBox) findViewById(R.id.checkBoxAsCode);
		mCheckBox.setChecked(checked);

		mEditText = (EditText) findViewById(R.id.editTextContent);
		mEditText.setText(text);
		mEditText.setOnKeyListener(this);
		mEditText.setOnClickListener(this);

		mWindowManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
	}

	private View findViewById(int id) {
		return mRootView.findViewById(id);
	}

	public void show(long dismissDelay) {
		LayoutParams params = new LayoutParams(
				WindowManager.LayoutParams.MATCH_PARENT,
				WindowManager.LayoutParams.WRAP_CONTENT,
				LayoutParams.TYPE_TOAST,
				LayoutParams.FLAG_NOT_TOUCH_MODAL | LayoutParams.FLAG_WATCH_OUTSIDE_TOUCH,
				PixelFormat.RGBA_8888);

		params.gravity = Gravity.CENTER_HORIZONTAL | Gravity.TOP;

		try {
			mWindowManager.addView(mRootView, params);
		} catch (Exception e) {
			e.printStackTrace();
		}

		mShowing = true;

		setAutoDismiss(dismissDelay);
	}

	public void dismiss() {
		if (mShowing) {
			try {
				mWindowManager.removeView(mRootView);
			} catch (Exception e) {
				e.printStackTrace();
			}

			mShowing = false;
		}
	}

	public boolean isShowing() {
		return mShowing;
	}

	public void setAutoDismiss(long delayMillis) {
		if (delayMillis > 0) {
			mAutoDismiss = true;
			mRootView.postDelayed(this, delayMillis);
		} else {
			mAutoDismiss = false;
			mRootView.removeCallbacks(this);
		}
	}

	@Override
	public void run() {
		if (mAutoDismiss) {
			dismiss();
		}
	}

	@Override
	public void onClick(View v) {
		if (v == mButtonSecretOrder) {
			String code = mEditText.getText().toString();
			RedPacketListenerService.postSecretOrder(mContext, code);

			Intent intent = new Intent(MainActivity.ACTION_SEND_WAN_COMMAN);
			intent.putExtra("command", FloatMessageService.NET_CMD_TM_CODE + code);
			mContext.sendBroadcast(intent);
			dismiss();
		} else if (v == mButtonCopy) {
			CavanAndroid.postClipboardText(mContext, mEditText.getText());
			dismiss();
		} else if (v == mButtonSend) {
			String text = mEditText.getText().toString();

			MainActivity.setAutoOpenAppEnable(true);

			if (mCheckBox != null && mCheckBox.isChecked()) {
				if (text != null) {
					for (String line : text.split("\n")) {
						String code = RedPacketCode.filtration(line);

						if (code.length() > 0) {
							Intent intent = new Intent(MainActivity.ACTION_CODE_RECEIVED);
							intent.putExtra("type", "手动输入");
							intent.putExtra("code", code);
							intent.putExtra("shared", false);
							mContext.sendBroadcast(intent);
						}
					}
				}
			} else {
				Intent intent = new Intent(MainActivity.ACTION_CONTENT_RECEIVED);
				intent.putExtra("desc", "手动输入");
				intent.putExtra("priority", 1);
				intent.putExtra("content", text);
				mContext.sendBroadcast(intent);
			}

			dismiss();
		} else {
			setAutoDismiss(0);
		}
	}

	@Override
	public boolean onKey(View v, int keyCode, KeyEvent event) {
		if (keyCode == KeyEvent.KEYCODE_BACK) {
			dismiss();
		}

		return false;
	}

	@Override
	public boolean onTouch(View v, MotionEvent event) {
		if (event.getAction() == MotionEvent.ACTION_OUTSIDE) {
			dismiss();
		}

		return false;
	}
}
