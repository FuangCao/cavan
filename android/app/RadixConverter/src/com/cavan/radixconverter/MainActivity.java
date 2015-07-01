package com.cavan.radixconverter;

import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.GridView;
import android.widget.Spinner;

public class MainActivity extends Activity implements TextWatcher, OnItemSelectedListener {

	private static final int MAX_COUNT_BIN = 32;
	private static final int MAX_COUNT_HEX = 8;
	private static final int MAX_COUNT_DEC = 10;
	private static final int MAX_COUNT_OCT = 11;

	private static final int[] PREFIX_BASE_MAP = { 10, 16, 2, 8 };

	private GridView mGridViewBin;
	private GridView mGridViewHex;
	private GridView mGridViewOct;
	private GridView mGridViewDec;
	private GridView mGridViewKeypad;
	private EditText mEditTextNum;
	private Spinner mSpinnerPrefix;

	private BitAdapter mAdapterBin;
	private BitAdapter mAdapterHex;
	private BitAdapter mAdapterDec;
	private BitAdapter mAdapterOct;
	private KeypadAdapter mAdapterKeypad;

	private long mValue;
	private BitView mCurrBitView;
	private List<BitAdapter> mAdapters = new ArrayList<BitAdapter>();

	private void setCurrBit(BitView view) {
		if (mCurrBitView != null) {
			mCurrBitView.setActive(false);
		}

		if (view != null) {
			if (mCurrBitView == null || view.getAdapter() != mCurrBitView.getAdapter()) {
				mAdapterKeypad.setKeyCount(view.getAdapter().getBase());
			}

			BitAdapter adapter = view.getAdapter();
			switch (adapter.getBase()) {
			case 8:
				mAdapterBin.setActiveViews(view.getIndex() * 3, 3);
				break;

			case 16:
				mAdapterBin.setActiveViews(view.getIndex() * 4, 4);
				break;

			default:
				if (mCurrBitView != null && mCurrBitView.getAdapter() != adapter) {
					mAdapterBin.setActiveViews(0, 0);
				}
				break;
			}

			view.setActive(true);
			updateValue(mValue, adapter.getBase());
		} else {
			mAdapterBin.setActiveViews(0, 0);
		}

		mCurrBitView = view;
	}

	private void updateValue(long value, int base) {
		setBase(base);
		mEditTextNum.setText(Long.toString(value, base));
	}

	private void updateValue(BitAdapter adapter, long value) {

		if (adapter != null) {
			value = adapter.getValue();
			updateValue(value, adapter.getBase());
		}

		if (value == mValue) {
			return;
		}

		for (BitAdapter adp : mAdapters) {
			if (adp != adapter) {
				adp.setValue(value);
			}
		}

		mValue = value;
	}

	private OnClickListener mBitClickListener = new OnClickListener() {

		@Override
		public void onClick(View v) {
			BitView view = (BitView) v;

			if (view == mCurrBitView) {
				view.add(1);
				updateValue(view.getAdapter(), 0);
			} else {
				setCurrBit((BitView) v);
			}
		}
	};

	private OnClickListener mKeypadClickListener = new OnClickListener() {

		@Override
		public void onClick(View v) {
			if (mCurrBitView == null) {
				return;
			}

			Button button = (Button) v;
			if (!button.getText().equals(mCurrBitView.getText())) {
				mCurrBitView.setText(button.getText());
				updateValue(mCurrBitView.getAdapter(), 0);
			}

			setCurrBit(mCurrBitView.getNextView());
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mGridViewBin = (GridView) findViewById(R.id.gridViewBin);
		mAdapterBin = new BitAdapter(getApplicationContext(), mBitClickListener, MAX_COUNT_BIN, 2);
		mAdapters.add(mAdapterBin);
		mGridViewBin.setAdapter(mAdapterBin);

		mGridViewHex = (GridView) findViewById(R.id.gridViewHex);
		mAdapterHex = new BitAdapter(getApplicationContext(), mBitClickListener, MAX_COUNT_HEX, 16);
		mAdapters.add(mAdapterHex);
		mGridViewHex.setAdapter(mAdapterHex);

		mGridViewDec = (GridView) findViewById(R.id.gridViewDec);
		mAdapterDec = new BitAdapter(getApplicationContext(), mBitClickListener, MAX_COUNT_DEC, 10);
		mAdapters.add(mAdapterDec);
		mGridViewDec.setAdapter(mAdapterDec);

		mGridViewOct = (GridView) findViewById(R.id.gridViewOct);
		mAdapterOct = new BitAdapter(getApplicationContext(), mBitClickListener, MAX_COUNT_OCT, 8);
		mAdapters.add(mAdapterOct);
		mGridViewOct.setAdapter(mAdapterOct);

		mGridViewKeypad = (GridView) findViewById(R.id.gridViewKeypad);
		mAdapterKeypad = new KeypadAdapter(getApplicationContext(), mKeypadClickListener);
		mGridViewKeypad.setAdapter(mAdapterKeypad);

		mEditTextNum = (EditText) findViewById(R.id.editTextNumber);
		mEditTextNum.addTextChangedListener(this);

		mSpinnerPrefix = (Spinner) findViewById(R.id.spinnerPrefix);
		String[] prefixList = getResources().getStringArray(R.array.num_prefix);
		ArrayAdapter<String> prefixAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, prefixList);
		mSpinnerPrefix.setAdapter(prefixAdapter);
		mSpinnerPrefix.setOnItemSelectedListener(this);
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
		int index = mSpinnerPrefix.getSelectedItemPosition();
		return PREFIX_BASE_MAP[index];
	}

	private void setBase(int base) {
		for (int i = 0; i < PREFIX_BASE_MAP.length; i++) {
			if (PREFIX_BASE_MAP[i] == base) {
				mSpinnerPrefix.setSelection(i);
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
			long value = Long.parseLong(mEditTextNum.getText().toString(), getBase());
			updateValue(null, value);
		} catch (Exception e) {
		}
	}

	@Override
	public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
		updateValue(mValue, getBase());
	}

	@Override
	public void onNothingSelected(AdapterView<?> arg0) {
	}
}
