package com.cavan.radixconverter;

import java.util.ArrayList;
import java.util.List;

import android.support.v7.app.ActionBarActivity;
import android.app.Activity;
import android.graphics.Color;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.GridView;
import android.widget.ListAdapter;
import android.widget.Spinner;
import android.widget.TableRow;
import android.widget.TextView;

public class MainActivity extends Activity {

	private static final int MAX_COUNT_BIN = 32;
	private static final int MAX_COUNT_HEX = 8;
	private static final int MAX_COUNT_DEC = 10;
	private static final int MAX_COUNT_OCT = 10;

	private long mValue = 100;
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

	private BitButton mButtonCurr;
	private List<BitAdapter> mAdapters = new ArrayList<BitAdapter>();

	private void alogd(String msg) {
		Log.e("Cavan", msg);
	}

	private OnClickListener mBitClickListener = new OnClickListener() {

		@Override
		public void onClick(View v) {
			BitButton button = (BitButton) v;

			if (button == mButtonCurr) {
				return;
			}

			if (mButtonCurr != null) {
				mButtonCurr.setActive(false);
			}

			mButtonCurr = button;
			button.setActive(true);

			mAdapterKeypad.setKeyCount(mButtonCurr.getAdapter().getBase());
		}
	};

	private OnClickListener mKeypadClickListener = new OnClickListener() {

		@Override
		public void onClick(View v) {
			if (mButtonCurr == null) {
				return;
			}

			Button button = (Button) v;
			if (button.getText().equals(mButtonCurr.getText())) {
				return;
			}

			mButtonCurr.setText(button.getText());
			long value = mButtonCurr.getAdapter().getValue();

			for (BitAdapter adapter : mAdapters) {
				if (adapter != mButtonCurr.getAdapter()) {
					adapter.setValue(value);
				}
			}
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
