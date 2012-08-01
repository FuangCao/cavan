package com.cavan.calculator2;

import android.app.Activity;
import android.os.Bundle;
import android.view.Menu;
import android.widget.EditText;
import android.widget.GridView;

public class MainActivity extends Activity
{
	private static int MAX_BIT_COUNT = 32;
	private EditText mEditText1;
	private GridView mGridView1;
	private BitMapAdapter mBitMapAdapter;
	private OnValueChangedListener mValueChangedListener = new OnValueChangedListener()
	{
		@Override
		public boolean OnValueChanged(long oldValue, long newValue)
		{
			mEditText1.setText(mBitMapAdapter.toString(2, 4));
			return true;
		}
	};

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mEditText1 = (EditText) findViewById(R.id.editText1);

		mGridView1 = (GridView) findViewById(R.id.gridView1);
		mGridView1.setNumColumns(8);
		mBitMapAdapter = new BitMapAdapter(this, 0, MAX_BIT_COUNT);
		mBitMapAdapter.setOnValueChangedListener(mValueChangedListener);
		mGridView1.setAdapter(mBitMapAdapter);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu)
	{
		getMenuInflater().inflate(R.menu.activity_main, menu);
		return true;
	}
}
