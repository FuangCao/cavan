package com.cavan.jwaootoyfactorytest;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.EditText;
import android.widget.Spinner;

import com.cavan.android.CavanKeyboardView;
import com.cavan.android.CavanMacAddressView;
import com.cavan.java.CavanMacAddress;

@SuppressLint("HandlerLeak")
public class MacConvertActivity extends Activity {

	private static final int MSG_UNLOCK = 1;
	private static final int MSG_START_ADDR_CHANGED = 2;
	private static final int MSG_END_ADDR_CHANGED = 3;
	private static final int MSG_NEXT_ADDR_CHANGED = 4;
	private static final int MSG_ADDR_COUNT_CHANGED = 5;
	private static final int MSG_PROJECT_CHANGED = 6;

	private CavanMacAddressView mAddressViewStart;
	private CavanMacAddressView mAddressViewEnd;
	private CavanMacAddressView mAddressViewNext;
	private EditText mEditTextAddressCount;
	private CavanKeyboardView mKeyboard;
	private Spinner mSpinnerProject;

	private Handler mHandler = new Handler() {

		private int mMessageBusy;

		private boolean setMessageBusy(int message) {
			if (mMessageBusy != message) {
				if (mMessageBusy != 0) {
					return false;
				}

				mMessageBusy = message;
			} else {
				removeMessages(MSG_UNLOCK);
			}

			sendEmptyMessageDelayed(MSG_UNLOCK, 100);

			return true;
		}

		public long updateAddressCount(CavanMacAddress address) {
			CavanMacAddress start = getAddressStart();

			long count;

			if (address.sub(start) == 0) {
				count = address.toLong();
			} else {
				count = 0;
			}

			mEditTextAddressCount.setText(Long.toString(count));

			return count;
		}

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case MSG_UNLOCK:
				mMessageBusy = 0;
				break;

			case MSG_PROJECT_CHANGED:
				if (setMessageBusy(msg.what)) {
					int position = mSpinnerProject.getSelectedItemPosition();
					if (position > 0) {
						CavanMacAddress address = getAddressStart();

						address.setByte(5, (byte) 0x88);

						if (position == 1) {
							address.setByte(4, (byte) 0xEA);
						} else {
							address.setByte(4, (byte) 0xEB);
						}

						mMessageBusy = MSG_START_ADDR_CHANGED;
						mAddressViewStart.setAddress(address);
					}
				}
				break;

			case MSG_START_ADDR_CHANGED:
			case MSG_ADDR_COUNT_CHANGED:
				if (setMessageBusy(msg.what)) {
					CavanMacAddress address = getAddressStart();

					long count = getAddressCount();
					if (count > 0) {
						address.add(getAddressCount());
						mAddressViewNext.setAddress(address);

						address.decrease();
						mAddressViewEnd.setAddress(address);
					} else {
						mAddressViewNext.setAddress(address);
						mAddressViewEnd.setAddress(address);
					}
				}
				break;

			case MSG_END_ADDR_CHANGED:
				if (setMessageBusy(msg.what)) {
					CavanMacAddress end = getAddressEnd();

					end.increase();
					mAddressViewNext.setAddress(end);

					updateAddressCount(end);
				}
				break;

			case MSG_NEXT_ADDR_CHANGED:
				if (setMessageBusy(msg.what)) {
					CavanMacAddress next = getAddressNext();
					if (next.isZero()) {
						mAddressViewEnd.setAddress(next);
					} else {
						mAddressViewEnd.setAddress(CavanMacAddress.sub(next, 1));
					}

					updateAddressCount(next);
				}
				break;
			}
		}
	};

	public CavanMacAddress getAddressStart() {
		return mAddressViewStart.getAddress();
	}

	public CavanMacAddress getAddressEnd() {
		return mAddressViewEnd.getAddress();
	}

	public CavanMacAddress getAddressNext() {
		return mAddressViewNext.getAddress();
	}

	public long getAddressCount() {
		String text = mEditTextAddressCount.getText().toString();
		if (text.length() > 0) {
			try {
				return Long.parseLong(text);
			} catch (NumberFormatException e) {
				e.printStackTrace();
			}
		}

		return 0;
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.mac_convert);

		mKeyboard = (CavanKeyboardView) findViewById(R.id.keyboardView);

		mAddressViewStart = (CavanMacAddressView) findViewById(R.id.macAddressViewStart);
		mAddressViewStart.setKeyboardView(mKeyboard);

		mAddressViewEnd = (CavanMacAddressView) findViewById(R.id.macAddressViewEnd);
		mAddressViewEnd.setKeyboardView(mKeyboard);

		mAddressViewNext = (CavanMacAddressView) findViewById(R.id.macAddressViewNext);
		mAddressViewNext.setKeyboardView(mKeyboard);

		mEditTextAddressCount = (EditText) findViewById(R.id.editTextAddrCount);
		mKeyboard.setupEditText(mEditTextAddressCount);

		mSpinnerProject = (Spinner) findViewById(R.id.spinnerProject);
		mSpinnerProject.setOnItemSelectedListener(new OnItemSelectedListener() {

			@Override
			public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
				mHandler.sendEmptyMessage(MSG_PROJECT_CHANGED);
			}

			@Override
			public void onNothingSelected(AdapterView<?> parent) {}
		});

		mAddressViewStart.addTextChangedListener(new TextWatcher() {

			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {}

			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

			@Override
			public void afterTextChanged(Editable s) {
				mHandler.sendEmptyMessage(MSG_START_ADDR_CHANGED);
			}
		});

		mAddressViewEnd.addTextChangedListener(new TextWatcher() {

			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {}

			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

			@Override
			public void afterTextChanged(Editable s) {
				mHandler.sendEmptyMessage(MSG_END_ADDR_CHANGED);
			}
		});

		mAddressViewNext.addTextChangedListener(new TextWatcher() {

			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {}

			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

			@Override
			public void afterTextChanged(Editable s) {
				mHandler.sendEmptyMessage(MSG_NEXT_ADDR_CHANGED);
			}
		});

		mEditTextAddressCount.addTextChangedListener(new TextWatcher() {

			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {}

			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

			@Override
			public void afterTextChanged(Editable s) {
				mHandler.sendEmptyMessage(MSG_ADDR_COUNT_CHANGED);
			}
		});
	}
}
