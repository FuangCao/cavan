package com.cavan.radixconverter;

import java.util.ArrayList;
import java.util.List;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnLongClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.GridView;
import android.widget.Spinner;

@SuppressLint("DefaultLocale")
public class MainActivity extends Activity implements TextWatcher, OnItemSelectedListener, OnClickListener {

	private static final int MAX_COUNT_BIN = 32;
	private static final int MAX_COUNT_HEX = 8;
	private static final int MAX_COUNT_DEC = 10;
	private static final int MAX_COUNT_OCT = 11;
	private static final long VALUE_MASK = (((long) 1) << MAX_COUNT_BIN) - 1;

	private static final int[] PREFIX_BASE_MAP = { 10, 16, 2, 8 };

	private GridView mGridViewBin;
	private GridView mGridViewHex;
	private GridView mGridViewOct;
	private GridView mGridViewDec;
	private GridView mGridViewKeypad;
	private EditText mEditTextNum;
	private Spinner mSpinnerRadix;

	private BitAdapter mAdapterBin;
	private BitAdapter mAdapterHex;
	private BitAdapter mAdapterDec;
	private BitAdapter mAdapterOct;
	private KeypadAdapter mAdapterKeypad;

	private long mValue;
	private BitView mCurrBitView;
	private List<BitAdapter> mAdapters = new ArrayList<BitAdapter>();

	private void setCurrBitView(BitView view) {
		BitAdapter currAdapter;

		if (mCurrBitView != null) {
			currAdapter = mCurrBitView.getAdapter();
			mCurrBitView.setActive(false);
		} else {
			currAdapter = null;
		}

		if (view != null) {
			BitAdapter adapter = view.getAdapter();

			if (adapter != currAdapter) {
				mAdapterKeypad.setNumKeyCount(view.getAdapter().getBase());
			}

			int width = view.getBitWidth();
			if (width < 0) {
				if (adapter != currAdapter) {
					mAdapterBin.setActiveViews(0, 0);
				}
			} else {
				mAdapterBin.setActiveViews(view.getOffset(), width);
			}

			view.setActive(true);
			updateValue(mValue, adapter.getBase());
		} else {
			mAdapterBin.setActiveViews(0, 0);
		}

		mCurrBitView = view;
		updateSelection();
	}

	private void updateSelection() {
		if (mCurrBitView == null) {
			return;
		}

		int position;
		Editable text = mEditTextNum.getText();
		int length = text.length();

		if (mCurrBitView.getIndex() < length) {
			position = length - mCurrBitView.getIndex();
		} else {
			position = 0;
		}

		mEditTextNum.setSelection(position);
	}

	private void updateValue(long value, int base) {
		if (base > 0) {
			setBase(base);
		} else {
			base = getBase();
		}

		if (value > 0) {
			mEditTextNum.setText(Long.toString(value, base).toUpperCase());
		} else {
			mEditTextNum.getText().clear();
		}

		updateSelection();
	}

	private void updateValue(Object obj, long value) {

		int base;

		if (obj != null) {
			if (obj instanceof BitAdapter) {
				BitAdapter adapter = (BitAdapter) obj;
				value = adapter.getValue();
				base = adapter.getBase();
			} else {
				base = getBase();
				try {
					Editable editable = mEditTextNum.getText();
					if (editable.length() > 0) {
						value = Long.parseLong(editable.toString(), base);
					} else {
						value = 0;
					}
				} catch (Exception e) {
					return;
				}
			}
		} else {
			base = -1;
		}

		if (value == mValue) {
			return;
		}

		mValue = value & VALUE_MASK;

		if (obj != mEditTextNum) {
			updateValue(mValue, base);
		}

		for (BitAdapter adp : mAdapters) {
			if (adp != obj) {
				adp.setValue(mValue);
			}
		}
	}

	private OnClickListener mBitClickListener = new OnClickListener() {

		@Override
		public void onClick(View v) {
			BitView view = (BitView) v;

			if (view == mCurrBitView) {
				view.add(1);
				updateValue(view.getAdapter(), 0);
			} else {
				setCurrBitView((BitView) v);
			}
		}
	};

	private OnLongClickListener mBitLongClickListener = new OnLongClickListener() {

		@Override
		public boolean onLongClick(View v) {
			BitView view = (BitView) v;
			if (view != mCurrBitView) {
				return false;
			}

			view.setValue(0);
			updateValue(view.getAdapter(), 0);

			return true;
		}
	};

	private OnClickListener mKeypadNumClickListener = new OnClickListener() {

		@Override
		public void onClick(View v) {
			Button button = (Button) v;

			if (mCurrBitView == null) {
				Editable text = mEditTextNum.getEditableText();
				text.insert(mEditTextNum.getSelectionStart(), button.getText());
				updateValue(mEditTextNum, 0);
			} else {
				if (!button.getText().equals(mCurrBitView.getText())) {
					mCurrBitView.setText(button.getText());
					updateValue(mCurrBitView.getAdapter(), 0);
				}

				setCurrBitView(mCurrBitView.getNextView());
			}
		}
	};

	private OnClickListener mKeypadCtrlClickListener = new OnClickListener() {

		@Override
		public void onClick(View v) {

			long value = mValue;
			Object obj = null;

			Button button = (Button) v;
			switch (button.getId()) {
			case 0:
				if (mCurrBitView == null) {
					int position = mEditTextNum.getSelectionStart();
					if (position > 0) {
						mEditTextNum.setSelection(position - 1);
					}
					return;
				} else if (mCurrBitView.getBitWidth() > 0) {
					value <<= mCurrBitView.getBitWidth();
					setCurrBitView(mCurrBitView.getPrevView());
				} else {
					value <<= 1;
				}
				break;

			case 2:
				if (mCurrBitView == null) {
					int position = mEditTextNum.getSelectionStart() + 1;
					if (position <= mEditTextNum.getText().length()) {
						mEditTextNum.setSelection(position);
					}
					return;
				} else if (mCurrBitView.getBitWidth() > 0) {
					value >>= mCurrBitView.getBitWidth();
					setCurrBitView(mCurrBitView.getNextView());
				} else {
					value >>= 1;
				}
				break;

			case 1:
				setCurrBitView(null);
				return;

			case 3:
				if (mCurrBitView != null && mCurrBitView.getMask() > 0) {
					value ^= mCurrBitView.getMask();
				} else {
					value = ~value;
				}
				break;

			case 4:
				Editable editable = mEditTextNum.getEditableText();
				int start = mEditTextNum.getSelectionStart();
				int end = mEditTextNum.getSelectionEnd();
				if (end <= start && start > 0) {
					start--;
				}
				editable.delete(start, end);
				obj = mEditTextNum;
				setCurrBitView(null);
				break;

			default:
				return;
			}

			updateValue(obj, value);
		}
	};

	private OnLongClickListener mKeypadCtrlLongClickListener = new OnLongClickListener() {

		@Override
		public boolean onLongClick(View v) {
			long value = mValue;
			Object obj = null;

			Button button = (Button) v;
			switch (button.getId()) {
			case 0:
				if (mCurrBitView == null || mCurrBitView.getBitWidth() < 1) {
					return false;
				}
				value <<= (MAX_COUNT_BIN - mCurrBitView.getOffset() - mCurrBitView.getBitWidth());
				setCurrBitView(mCurrBitView.getAdapter().getLastView());
				break;

			case 2:
				if (mCurrBitView == null || mCurrBitView.getBitWidth() < 1) {
					return false;
				}
				value >>= mCurrBitView.getOffset();
				setCurrBitView(mCurrBitView.getAdapter().getFirstView());
				break;

			case 3:
				value = ~value;
				break;

			case 4:
				mEditTextNum.getText().clear();
				obj = mEditTextNum;
				setCurrBitView(null);
				break;

			default:
				return false;
			}

			updateValue(obj, value);

			return true;
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mGridViewBin = (GridView) findViewById(R.id.gridViewBin);
		mAdapterBin = new BitAdapter(mGridViewBin, mBitClickListener, mBitLongClickListener, MAX_COUNT_BIN, 2);
		mAdapters.add(mAdapterBin);

		mGridViewHex = (GridView) findViewById(R.id.gridViewHex);
		mAdapterHex = new BitAdapter(mGridViewHex, mBitClickListener, mBitLongClickListener, MAX_COUNT_HEX, 16);
		mAdapters.add(mAdapterHex);

		mGridViewDec = (GridView) findViewById(R.id.gridViewDec);
		mAdapterDec = new BitAdapter(mGridViewDec, mBitClickListener, mBitLongClickListener, MAX_COUNT_DEC, 10);
		mAdapters.add(mAdapterDec);

		mGridViewOct = (GridView) findViewById(R.id.gridViewOct);
		mAdapterOct = new BitAdapter(mGridViewOct, mBitClickListener, mBitLongClickListener, MAX_COUNT_OCT, 8);
		mAdapters.add(mAdapterOct);

		mGridViewKeypad = (GridView) findViewById(R.id.gridViewKeypad);
		mAdapterKeypad = new KeypadAdapter(mGridViewKeypad, mKeypadNumClickListener, mKeypadCtrlClickListener, mKeypadCtrlLongClickListener);

		mEditTextNum = (EditText) findViewById(R.id.editTextNumber);
		mEditTextNum.addTextChangedListener(this);
		mEditTextNum.setOnClickListener(this);

		mSpinnerRadix = (Spinner) findViewById(R.id.spinnerRadix);
		String[] radixList = getResources().getStringArray(R.array.radix_names);
		ArrayAdapter<String> radixAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, radixList);
		mSpinnerRadix.setAdapter(radixAdapter);
		mSpinnerRadix.setOnItemSelectedListener(this);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}

	private int getBase() {
		int index = mSpinnerRadix.getSelectedItemPosition();
		return PREFIX_BASE_MAP[index];
	}

	private void setBase(int base) {
		for (int i = 0; i < PREFIX_BASE_MAP.length; i++) {
			if (PREFIX_BASE_MAP[i] == base) {
				mSpinnerRadix.setSelection(i);
				break;
			}
		}
	}

	@Override
	public void afterTextChanged(Editable s) {
	}

	@Override
	public void beforeTextChanged(CharSequence s, int start, int count, int after) {
	}

	@Override
	public void onTextChanged(CharSequence s, int start, int before, int count) {
		try {
			updateValue(mEditTextNum, 0);
		} catch (Exception e) {
		}
	}

	@Override
	public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
		updateValue(mValue, -1);
		if (mCurrBitView == null) {
			mAdapterKeypad.setNumKeyCount(getBase());
		}
	}

	@Override
	public void onNothingSelected(AdapterView<?> arg0) {
	}

	@Override
	public void onClick(View v) {
		setCurrBitView(null);
		mAdapterKeypad.setNumKeyCount(getBase());
	}
}
