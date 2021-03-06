package com.cavan.cavanmain;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.graphics.PixelFormat;
import android.os.Build;
import android.os.Handler;
import android.text.Editable;
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

import com.cavan.accessibility.CavanRedPacketAlipay;
import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanEditText;
import com.cavan.android.CavanEditText.OnSelectionChangedListener;
import com.cavan.java.RedPacketFinder;

public class FloatEditorDialog implements OnClickListener, Runnable, OnKeyListener, OnTouchListener, OnSelectionChangedListener {

	private static FloatEditorDialog mInstance;

	private Context mContext;
	private boolean mShowing;
	private boolean mAutoDismiss;

	private View mRootView;
	private Button mButtonCopy;
	private Button mButtonSend;
	private Button mButtonExtract;
	private Button mButtonReserve;
	private Button mButtonDeleteHead;
	private Button mButtonDeleteTail;

	private CheckBox mCheckBox;
	private CavanEditText mEditText;

	private Dialog mDialog;
	private WindowManager mWindowManager;
	private Handler mHandler = new Handler();

	public static FloatEditorDialog getInstance(Context context, CharSequence text, boolean checked, boolean copy) {
		if (mInstance == null) {
			mInstance = new FloatEditorDialog(context.getApplicationContext());
		}

		mInstance.updateContent(text, checked, copy);

		return mInstance;
	}

	private FloatEditorDialog(Context context) {
		mContext = context;

		mRootView = View.inflate(context, R.layout.float_editor, null);
		mRootView.setOnTouchListener(this);

		mButtonSend = (Button) findViewById(R.id.buttonSend);
		mButtonSend.setOnClickListener(this);

		mButtonCopy = (Button) findViewById(R.id.buttonCopy);
		mButtonCopy.setOnClickListener(this);

		mButtonExtract = (Button) findViewById(R.id.buttonExtract);
		mButtonExtract.setOnClickListener(this);

		mButtonReserve = (Button) findViewById(R.id.buttonReserve);
		mButtonReserve.setOnClickListener(this);

		mButtonDeleteHead = (Button) findViewById(R.id.buttonDeleteHead);
		mButtonDeleteHead.setOnClickListener(this);

		mButtonDeleteTail = (Button) findViewById(R.id.buttonDeleteTail);
		mButtonDeleteTail.setOnClickListener(this);

		mCheckBox = (CheckBox) findViewById(R.id.checkBoxAsCode);

		mEditText = (CavanEditText) findViewById(R.id.editTextContent);
		mEditText.setOnKeyListener(this);
		mEditText.setOnClickListener(this);
		mEditText.setOnSelectionChangedListener(this);

		mWindowManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
	}

	public void updateContent(CharSequence text, boolean checked, boolean copy) {
		mCheckBox.setChecked(checked);
		mEditText.setText(text);
		mButtonCopy.setText(copy ? R.string.copy : R.string.share);

		mButtonReserve.setVisibility(View.GONE);
		mButtonDeleteHead.setVisibility(View.GONE);
		mButtonDeleteTail.setVisibility(View.GONE);
	}

	private View findViewById(int id) {
		return mRootView.findViewById(id);
	}

	private LayoutParams createLayoutParams(int flags) {
		LayoutParams params = new LayoutParams(
				WindowManager.LayoutParams.MATCH_PARENT,
				WindowManager.LayoutParams.WRAP_CONTENT,
				CavanAndroid.getAlertWindowType(),
				flags,
				PixelFormat.RGBA_8888);

		params.gravity = Gravity.CENTER_HORIZONTAL | Gravity.TOP;

		return params;
	}

	public void show(long dismissDelay) {
		if (Build.VERSION.SDK_INT < Build.VERSION_CODES.KITKAT) {
			if (mDialog == null) {
				AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
				builder.setView(mRootView);
				builder.setCancelable(true);
				mDialog = builder.create();
				mDialog.getWindow().setAttributes(createLayoutParams(0));
			}

			mDialog.show();
		} else if (!mShowing) {
			mShowing = true;

			try {
				mWindowManager.addView(mRootView, createLayoutParams(LayoutParams.FLAG_NOT_TOUCH_MODAL | LayoutParams.FLAG_WATCH_OUTSIDE_TOUCH));
			} catch (Exception e) {
				e.printStackTrace();
			}
		}

		setAutoDismiss(dismissDelay);
	}

	public void dismiss() {
		if (mDialog != null) {
			mDialog.dismiss();
		} else if (mShowing) {
			try {
				mWindowManager.removeView(mRootView);
			} catch (Exception e) {
				e.printStackTrace();
			}
		}

		mShowing = false;
	}

	public boolean isShowing() {
		if (mDialog != null) {
			return mDialog.isShowing();
		}

		return mShowing;
	}

	public void setAutoDismiss(long delayMillis) {
		mHandler.removeCallbacks(this);

		if (delayMillis > 0) {
			mAutoDismiss = true;
			mHandler.postDelayed(this, delayMillis);
		} else {
			mAutoDismiss = false;
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
		setAutoDismiss(0);

		if (v == mButtonExtract) {
			String text = RedPacketFinder.getRedPacketDigitCode(mEditText.getText().toString(), false);
			if (text == null) {
				dismiss();
				return;
			}

			int length = text.length();

			CavanAndroid.dLog("text[" + length + "] = " + text);

			if (length > 0) {
				if (length % 8 == 0) {
					RedPacketListenerService listener = RedPacketListenerService.instance;
					if (listener != null) {
						listener.addRedPacketCodes(RedPacketFinder.splitRedPacketDigitCodes(text), "手动提取", false);
					}
					dismiss();
				} else {
					mEditText.setText(text);
				}
			}
		} else if (v == mButtonCopy) {
			String text = mEditText.getText().toString();

			CavanAndroid.postClipboardTextTemp(mContext, text);

			FloatMessageService service = FloatMessageService.instance;
			if (service != null) {
				service.sendWanCommand(FloatMessageService.NET_CMD_CLIPBOARD + text.replace('\n', ' '));
			}

			dismiss();
		} else if (v == mButtonSend) {
			String text = mEditText.getText().toString();

			CavanMessageActivity.setAutoOpenAppEnable(true);

			if (mCheckBox != null && mCheckBox.isChecked()) {
				if (text != null) {
					for (String line : text.split("\n")) {
						String code = CavanRedPacketAlipay.filtration(line);

						if (code.length() > 0) {
							RedPacketListenerService listener = RedPacketListenerService.instance;
							if (listener != null) {
								listener.addRedPacketCode(code, "手动输入", true);
							}
						}
					}
				}
			} else {
				RedPacketListenerService listener = RedPacketListenerService.instance;
				if (listener != null) {
					listener.addRedPacketCode(text, "手动输入", true);
				}
			}

			dismiss();
		} else {
			Editable editable = mEditText.getEditableText();
			int start = mEditText.getSelectionStart();
			int end = mEditText.getSelectionEnd();

			if (start > end) {
				int bak = end;
				end = start;
				start = bak;
			}

			try {
				if (v == mButtonReserve) {
					if (start != end) {
						editable.replace(0, editable.length(), editable.subSequence(start, end));
					}
				} else if (v == mButtonDeleteHead) {
					editable.delete(0, start);
				} else if (v == mButtonDeleteTail) {
					editable.delete(end, editable.length());
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
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

	@Override
	public void onSelectionChanged(int start, int end) {
		if (start != end) {
			mButtonReserve.setVisibility(View.VISIBLE);
		} else {
			mButtonReserve.setVisibility(View.GONE);
		}

		if (start > 0) {
			mButtonDeleteHead.setVisibility(View.VISIBLE);
			mButtonDeleteTail.setVisibility(View.VISIBLE);
		}
	}
}
