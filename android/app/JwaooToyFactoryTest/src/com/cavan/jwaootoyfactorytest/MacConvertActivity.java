package com.cavan.jwaootoyfactorytest;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.res.Resources;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanBusyLock;
import com.cavan.android.CavanMacAddressView;
import com.cavan.java.CavanMacAddress;
import com.cavan.resource.CavanKeyboardViewNumber;

@SuppressLint("HandlerLeak")
public class MacConvertActivity extends Activity {

	public static final CavanMacAddress sAddressStartModel06 = new CavanMacAddress().fromString("88:EA:00:00:00:00");
	public static final CavanMacAddress sAddressStartModel10 = new CavanMacAddress().fromString("88:EB:00:00:00:00");
	public static final CavanMacAddress sAddressStartModel01 = new CavanMacAddress().fromString("88:EC:00:00:00:00");
	public static final CavanMacAddress sAddressStartModel11 = new CavanMacAddress().fromString("88:ED:00:00:00:00");
	public static final CavanMacAddress sAddressStartModel03 = new CavanMacAddress().fromString("88:EE:00:00:00:00");

	public static final CavanMacAddress sAddressStartS1 = new CavanMacAddress().fromString("89:F1:00:00:00:00");
	public static final CavanMacAddress sAddressStartT1 = new CavanMacAddress().fromString("89:F2:00:00:00:00");

	private static final int MSG_START_ADDR_CHANGED = 2;
	private static final int MSG_END_ADDR_CHANGED = 3;
	private static final int MSG_NEXT_ADDR_CHANGED = 4;
	private static final int MSG_ADDR_COUNT_CHANGED = 5;
	private static final int MSG_PROJECT_CHANGED = 6;

	private static final int PROJECT_INDEX_NONE = 0;
	private static final int PROJECT_INDEX_MODEL06 = 1;
	private static final int PROJECT_INDEX_MODEL10 = 2;
	private static final int PROJECT_INDEX_MODEL01 = 3;
	private static final int PROJECT_INDEX_MODEL03 = 4;
	private static final int PROJECT_INDEX_MODEL11 = 5;
	private static final int PROJECT_INDEX_S1 = 6;
	private static final int PROJECT_INDEX_T1 = 7;

	private CavanMacAddressView mAddressViewStart;
	private CavanMacAddressView mAddressViewEnd;
	private CavanMacAddressView mAddressViewNext;
	private EditText mEditTextAddressCount;
	private CavanKeyboardViewNumber mKeyboard;
	private Spinner mSpinnerProject;
	private Button mButtonCopy;

	private CavanBusyLock mLock = new CavanBusyLock(200);

	private Handler mHandler = new Handler() {

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
			case MSG_PROJECT_CHANGED:
				int position = mSpinnerProject.getSelectedItemPosition();
				if (position > PROJECT_INDEX_NONE) {
					CavanMacAddress address = getAddressStart();

					if (position == PROJECT_INDEX_MODEL06) {
						address.setPrefix(sAddressStartModel06);
					} else if (position == PROJECT_INDEX_MODEL10) {
						address.setPrefix(sAddressStartModel10);
					} else if (position == PROJECT_INDEX_MODEL01) {
						address.setPrefix(sAddressStartModel01);
					} else if (position == PROJECT_INDEX_MODEL11) {
						address.setPrefix(sAddressStartModel11);
					} else if (position == PROJECT_INDEX_MODEL03) {
						address.setPrefix(sAddressStartModel03);
					} else if (position == PROJECT_INDEX_S1) {
						address.setPrefix(sAddressStartS1);
					} else if (position == PROJECT_INDEX_T1) {
						address.setPrefix(sAddressStartT1);
					} else {
						CavanAndroid.showToast(getApplicationContext(), "Invalid project！");
					}

					mLock.setOwner(mAddressViewStart);
					mAddressViewStart.setAddress(address);
				}
				break;

			case MSG_START_ADDR_CHANGED:
			case MSG_ADDR_COUNT_CHANGED:
				CavanMacAddress address = getAddressStart();

				if (address.startsWith(sAddressStartModel06)) {
					mSpinnerProject.setSelection(PROJECT_INDEX_MODEL06);
				} else if (address.startsWith(sAddressStartModel10)) {
					mSpinnerProject.setSelection(PROJECT_INDEX_MODEL10);
				} else if (address.startsWith(sAddressStartModel11)) {
					mSpinnerProject.setSelection(PROJECT_INDEX_MODEL11);
				} else if (address.startsWith(sAddressStartModel03)) {
					mSpinnerProject.setSelection(PROJECT_INDEX_MODEL03);
				} else if (address.startsWith(sAddressStartS1)) {
					mSpinnerProject.setSelection(PROJECT_INDEX_S1);
				} else if (address.startsWith(sAddressStartT1)) {
					mSpinnerProject.setSelection(PROJECT_INDEX_T1);
				} else {
					mSpinnerProject.setSelection(PROJECT_INDEX_NONE);
				}

				address.add(getAddressCount());
				mAddressViewNext.setAddress(address);

				address.decrease();
				mAddressViewEnd.setAddress(address);
				break;

			case MSG_END_ADDR_CHANGED:
				CavanMacAddress end = getAddressEnd();

				end.increase();
				mAddressViewNext.setAddress(end);

				updateAddressCount(end);
				break;

			case MSG_NEXT_ADDR_CHANGED:
				CavanMacAddress next = getAddressNext();

				mAddressViewEnd.setAddress(next.decreaseMacAddress());
				updateAddressCount(next);
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

		mKeyboard = (CavanKeyboardViewNumber) findViewById(R.id.keyboardView);

		mAddressViewStart = (CavanMacAddressView) findViewById(R.id.macAddressViewStart);
		mAddressViewStart.setKeyboardView(mKeyboard, CavanKeyboardViewNumber.KEYBOARD_RADIX16);

		mAddressViewEnd = (CavanMacAddressView) findViewById(R.id.macAddressViewEnd);
		mAddressViewEnd.setKeyboardView(mKeyboard, CavanKeyboardViewNumber.KEYBOARD_RADIX16);

		mAddressViewNext = (CavanMacAddressView) findViewById(R.id.macAddressViewNext);
		mAddressViewNext.setKeyboardView(mKeyboard, CavanKeyboardViewNumber.KEYBOARD_RADIX16);

		mEditTextAddressCount = (EditText) findViewById(R.id.editTextAddrCount);
		mKeyboard.setupEditText(mEditTextAddressCount);
		mKeyboard.setKeyboard(mEditTextAddressCount, CavanKeyboardViewNumber.KEYBOARD_RADIX10);

		mSpinnerProject = (Spinner) findViewById(R.id.spinnerProject);
		mSpinnerProject.setOnItemSelectedListener(new OnItemSelectedListener() {

			@Override
			public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
				if (mLock.acquire(mSpinnerProject)) {
					mHandler.sendEmptyMessage(MSG_PROJECT_CHANGED);
				}
			}

			@Override
			public void onNothingSelected(AdapterView<?> parent) {}
		});

		mButtonCopy = (Button) findViewById(R.id.buttonCopy);
		mButtonCopy.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				StringBuilder builder = new StringBuilder();
				Resources resources = getResources();

				if (mSpinnerProject.getSelectedItemPosition() > 0) {
					builder.append(resources.getString(R.string.project_name));
					builder.append(mSpinnerProject.getSelectedItem()).append('\n');
				}

				builder.append(resources.getString(R.string.start_mac_address));
				builder.append(getAddressStart()).append('\n');
				builder.append(resources.getString(R.string.end_mac_address));
				builder.append(getAddressEnd()).append('\n');
				builder.append(resources.getString(R.string.address_count));
				builder.append(getAddressCount());

				CavanAndroid.postClipboardText(getApplicationContext(), builder);
				CavanAndroid.showToast(getApplicationContext(), R.string.prompt_copied_to_clipboard);
			}
		});

		mAddressViewStart.addTextChangedListener(new TextWatcher() {

			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {}

			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

			@Override
			public void afterTextChanged(Editable s) {
				if (mLock.acquire(mAddressViewStart)) {
					mHandler.sendEmptyMessage(MSG_START_ADDR_CHANGED);
				}
			}
		});

		mAddressViewEnd.addTextChangedListener(new TextWatcher() {

			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {}

			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

			@Override
			public void afterTextChanged(Editable s) {
				if (mLock.acquire(mAddressViewEnd)) {
					mHandler.sendEmptyMessage(MSG_END_ADDR_CHANGED);
				}
			}
		});

		mAddressViewNext.addTextChangedListener(new TextWatcher() {

			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {}

			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

			@Override
			public void afterTextChanged(Editable s) {
				if (mLock.acquire(mAddressViewNext)) {
					mHandler.sendEmptyMessage(MSG_NEXT_ADDR_CHANGED);
				}
			}
		});

		mEditTextAddressCount.addTextChangedListener(new TextWatcher() {

			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {}

			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

			@Override
			public void afterTextChanged(Editable s) {
				if (mLock.acquire(mEditTextAddressCount)) {
					mHandler.sendEmptyMessage(MSG_ADDR_COUNT_CHANGED);
				}
			}
		});
	}
}
