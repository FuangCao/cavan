package com.cavan.radixconverter;

import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.GridView;

public class MainActivity extends Activity {

	private static final int MAX_COUNT_BIN = 32;
	private static final int MAX_COUNT_HEX = 8;
	private static final int MAX_COUNT_DEC = 10;
	private static final int MAX_COUNT_OCT = 10;

	private GridView mGridViewBin;
	private GridView mGridViewHex;
	private GridView mGridViewOct;
	private GridView mGridViewDec;
	private GridView mGridViewKeypad;

	private BitAdapter mAdapterBin;
	private BitAdapter mAdapterHex;
	private BitAdapter mAdapterDec;
	private BitAdapter mAdapterOct;
	private KeypadAdapter mAdapterKeypad;

	private BitView mCurrBitView;
	private List<BitAdapter> mAdapters = new ArrayList<BitAdapter>();

	private void setCurrBit(BitView button) {
		if (mCurrBitView != null) {
			mCurrBitView.setActive(false);
		}

		if (button != null) {
			button.setActive(true);

			if (mCurrBitView == null || button.getAdapter() != mCurrBitView.getAdapter()) {
				mAdapterKeypad.setKeyCount(button.getAdapter().getBase());
			}
		}

		mCurrBitView = button;
	}

	private void updateValue(BitAdapter adapter) {

		long value = adapter.getValue();

		for (BitAdapter adp : mAdapters) {
			if (adp != adapter) {
				adp.setValue(value);
			}
		}
	}

	private OnClickListener mBitClickListener = new OnClickListener() {

		@Override
		public void onClick(View v) {
			BitView button = (BitView) v;

			if (button == mCurrBitView) {
				button.add(1);
				updateValue(button.getAdapter());
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
				updateValue(mCurrBitView.getAdapter());
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
}
