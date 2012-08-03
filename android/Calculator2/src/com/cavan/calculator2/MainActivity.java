package com.cavan.calculator2;

import android.app.Activity;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.Menu;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.GridView;
import android.widget.Spinner;
import android.widget.TextView;

public class MainActivity extends Activity
{
	private EditText mEditTextFormula;
	private EditText mEditTextResult;

	private Spinner mSpinnerResult;

	private BitmapGridView mBitmapGridView;
	private GridView mGridViewKeypad;

	private KeypadAdapter mKeypadAdapter;

	private int mBase;

	private OnValueChangedListener mValueChangedListener = new OnValueChangedListener()
	{
		@Override
		public boolean OnValueChanged(long oldValue, long newValue)
		{
			mEditTextResult.setText(mBitmapGridView.toString());
			return true;
		}
	};

	private OnKeypadChangedListener mKeypadChangedListener = new OnKeypadChangedListener()
	{
		@Override
		public void OnTextKeyClicked(Button button)
		{
			mEditTextFormula.append(button.getText().toString());
		}

		@Override
		public void OnCleanKeyClicked(Button button)
		{
			mEditTextFormula.setText("");
		}

		@Override
		public void OnBackKeyClicked(Button button)
		{
			CharSequence text = mEditTextFormula.getText();
			int length = text.length();
			if (length > 0)
			{
				mEditTextFormula.setText(text.subSequence(0, length - 1));
			}
		}
	};

	private BaseAdapter mAdapterSpinnerResult = new BaseAdapter()
	{
		@Override
		public View getView(int position, View convertView, ViewGroup parent)
		{
			TextView view = new TextView(MainActivity.this);
			view.setText(Integer.toString(position + 2));
			view.setTextSize(30);

			return view;
		}

		@Override
		public long getItemId(int position)
		{
			return 0;
		}

		@Override
		public Object getItem(int position)
		{
			return null;
		}

		@Override
		public int getCount()
		{
			return 26 + 10 - 2;
		}
	};

	private OnItemSelectedListener mItemSelectedListenerSpinnerResult = new OnItemSelectedListener()
	{
		@Override
		public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3)
		{
			TextView view = (TextView) arg1;
			mBase = Integer.decode(view.getText().toString());
			mEditTextResult.setText(mBitmapGridView.toString(mBase));
		}

		@Override
		public void onNothingSelected(AdapterView<?> arg0)
		{
			arg0.setSelection(10 - 2);
		}
	};

	private TextWatcher mTextWatcherFormula = new TextWatcher()
	{
		@Override
		public void onTextChanged(CharSequence s, int start, int before, int count)
		{
			mBitmapGridView.setValue(s.toString(), 10);
			mEditTextResult.setText(mBitmapGridView.toString(mBase));
		}

		@Override
		public void beforeTextChanged(CharSequence s, int start, int count, int after)
		{
		}

		@Override
		public void afterTextChanged(Editable s)
		{
		}
	};

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mEditTextFormula = (EditText) findViewById(R.id.editTextFormula);
		mEditTextFormula.addTextChangedListener(mTextWatcherFormula);

		mEditTextResult = (EditText) findViewById(R.id.editTextResult);

		mSpinnerResult = (Spinner) findViewById(R.id.spinnerResult);
		mSpinnerResult.setAdapter(mAdapterSpinnerResult);
		mSpinnerResult.setOnItemSelectedListener(mItemSelectedListenerSpinnerResult);
		mSpinnerResult.setSelection(10 - 2);

		mBitmapGridView = (BitmapGridView) findViewById(R.id.gridViewBitMap);
		mBitmapGridView.setOnValueChangedListener(mValueChangedListener);

		mGridViewKeypad = (GridView) findViewById(R.id.gridViewKeypad);
		mKeypadAdapter = new KeypadAdapter(this);
		mKeypadAdapter.setOnKeypadChangedListener(mKeypadChangedListener);
		mGridViewKeypad.setAdapter(mKeypadAdapter);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu)
	{
		getMenuInflater().inflate(R.menu.activity_main, menu);
		return true;
	}
}
