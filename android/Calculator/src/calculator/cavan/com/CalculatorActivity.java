package calculator.cavan.com;

import android.app.Activity;
import android.graphics.Color;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.Gravity;
import android.view.View;
import android.view.View.OnFocusChangeListener;
import android.view.ViewGroup;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.LinearLayout.LayoutParams;
import android.widget.Spinner;
import android.widget.TextView;

public class CalculatorActivity extends Activity
{
	private Button mButtonBits[] = new Button[32];
	private Button mButtonKeys[];
	private Button mButtonLeft;
	private Button mButtonRight;
	private Button mButtonDel;
	private Button mButtonClr;

	private EditText mEditTextTotal;
	private EditText mEditTextPart;
	private EditText mEditTextShift;

	private Spinner mSpinnerStart;
	private Spinner mSpinnerEnd;
	private Spinner mSpinnerLength;
	private Spinner mSpinnerBaseTotal;
	private Spinner mSpinnerBasePart;

	private int mStart = 0;
	private int mLength = 32;
	private int mBaseTotal = 10;
	private int mBasePart = 10;

	private TextWatcher mTextWatcherTotal;
	private TextWatcher mTextWatcherPart;

	private boolean mEventPending = false;

	private long mTotalValue;
	private long mPartValue;

	private void SetKeyStates(int base)
	{
		int i;

		if (base > mButtonKeys.length)
		{
			base = mButtonKeys.length;
		}

		for (i = 0; i < base; i++)
		{
			mButtonKeys[i].setEnabled(true);
		}

		for (base = mButtonKeys.length; i < base; i++)
		{
			mButtonKeys[i].setEnabled(false);
		}
	}

	private int CharToValue(char c)
	{
		if (c >= '0' && c <= '9')
		{
			return c - '0';
		}
		else if (c >= 'a' && c <= 'z')
		{
			return c - 'a' + 10;
		}
		else if (c >= 'A' && c <= 'Z')
		{
			return c - 'A' + 10;
		}
		else
		{
			return -1;
		}
	}

	private long StringToValue(String string, int base)
	{
		double value = 0;
		int length = string.length();

		for (int i = 0; i < length; i++)
		{
			int temp = CharToValue(string.charAt(i));

			if (temp > base || temp < 0)
			{
				return 0;
			}

			value = value * base + temp;
		}

		return (long) value;
	}

	private long BitsToValue(int start, int length)
	{
		long value = 0;

		for (int i = 0; i < length; i++)
		{
			CharSequence charSequence = mButtonBits[i + start].getText();

			if (charSequence.charAt(0) == '1')
			{
				value |= 1 << i;
			}
		}

		return value;
	}

	private void ValueToBits(long value, int start, int length)
	{
		for (int i = 0; i < length; i++)
		{
			if ((value & (1 << i)) == 0)
			{
				mButtonBits[start + i].setText("0");
			}
			else
			{
				mButtonBits[start + i].setText("1");
			}
		}
	}

	private char ValueToChar(long value)
	{
		return (char) (value < 10 ? value + '0' : value - 10 + 'A');
	};

	private int ValueToStringBits(long value, char buff[], int bits)
	{
		int mask;
		int i;

		if (bits == 0)
		{
			return 0;
		}

		mask = (1 << bits) - 1;

		for (i = 0; i < 32; i += bits)
		{
			buff[i / bits] = ValueToChar((value >> i) & mask);
		}

		return i / bits;
	}

	private int ValueToStringBase(long value, char buff[], int base)
	{
		int length;
		double db_value;

		if (value < 0)
		{
			db_value = (value & 0x7FFFFFFF) + 2147483648.0;
		}
		else
		{
			db_value = value;
		}

		for (length = 0; db_value >= 1; length++)
		{
			buff[length] = ValueToChar((long) (db_value % base));
			db_value /= base;
		}

		return length;
	}

	private String ValueToString(long value, int base)
	{
		char buff[] = new char[1024];
		int length;

		switch (base)
		{
		case 2:
			length = ValueToStringBits(value, buff, 1);
			break;

		case 4:
			length = ValueToStringBits(value, buff, 2);
			break;

		case 8:
			length = ValueToStringBits(value, buff, 3);
			break;

		case 16:
			length = ValueToStringBits(value, buff, 4);
			break;

		case 32:
			length = ValueToStringBits(value, buff, 5);
			break;

		default:
			length = ValueToStringBase(value, buff, base);
		}

		if (length == 0)
		{
			return new String("0");
		}

		StringBuilder builder = new StringBuilder(length);

		for (int i = length - 1; i >= 0; i--)
		{
			builder.append(buff[i]);
		}

		return builder.toString();
	}

	private void UpdateTextViewPartText()
	{
		mPartValue = GetPartValue(mTotalValue, mStart, mLength);
		mEditTextPart.setText(ValueToString(mPartValue, mBasePart));
		mEditTextPart.setSelection(mEditTextPart.length());
	}

	private void UpdateTextViewTotalText()
	{
		mTotalValue = BitsToValue(0, 32);
		mEditTextTotal.setText(ValueToString(mTotalValue, mBaseTotal));
		mEditTextTotal.setSelection(mEditTextTotal.length());
	}

	private void UpdateTextViewTotalText(long value)
	{
		mTotalValue = value;
		ValueToBits(value, mStart, mLength);
		mEditTextTotal.setText(ValueToString(mTotalValue, mBaseTotal));
		mEditTextTotal.setSelection(mEditTextTotal.length());
	}

	private long GetPartValue(long value, int start, int length)
	{
		long tmp_value = 0;

		for (int i = 0; i < length; i++)
		{
			if ((value & (1 << (start + i))) != 0)
			{
				tmp_value |= 1 << i;
			}
		}

		return tmp_value;
	}

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);

		LinearLayout layoutRoot = (LinearLayout) findViewById(R.id.linearLayout1);

		mEditTextTotal = (EditText) findViewById(R.id.editText1);
		mTextWatcherTotal = new TextWatcher()
		{

			@Override
			public void onTextChanged(CharSequence s, int start, int before,
					int count)
			{
				// TODO Auto-generated method stub

			}

			@Override
			public void beforeTextChanged(CharSequence s, int start, int count,
					int after)
			{
				// TODO Auto-generated method stub

			}

			@Override
			public void afterTextChanged(Editable s)
			{
				// TODO Auto-generated method stub
				if (mEventPending)
				{
					return;
				}

				mEventPending = true;

				mTotalValue = StringToValue(mEditTextTotal.getText().toString(), mBaseTotal);

				ValueToBits(mTotalValue, 0, 32);
				UpdateTextViewPartText();

				mEventPending = false;
			}
		};

		mEditTextTotal.addTextChangedListener(mTextWatcherTotal);
		mEditTextTotal.setOnFocusChangeListener(new OnFocusChangeListener()
		{

			@Override
			public void onFocusChange(View v, boolean hasFocus)
			{
				// TODO Auto-generated method stub
				if (hasFocus)
				{
					SetKeyStates(mBaseTotal);
				}
			}
		});

		mEditTextPart = (EditText) findViewById(R.id.editText2);
		mTextWatcherPart = new TextWatcher()
		{

			@Override
			public void onTextChanged(CharSequence s, int start, int before,
					int count)
			{
				// TODO Auto-generated method stub

			}

			@Override
			public void beforeTextChanged(CharSequence s, int start, int count,
					int after)
			{
				// TODO Auto-generated method stub

			}

			@Override
			public void afterTextChanged(Editable s)
			{
				// TODO Auto-generated method stub
				if (mEventPending)
				{
					return;
				}

				mEventPending = true;

				long value = StringToValue(mEditTextPart.getText().toString(),
						mBasePart);
				ValueToBits(value, mStart, mLength);
				UpdateTextViewTotalText();

				mEventPending = false;
			}
		};

		mEditTextPart.addTextChangedListener(mTextWatcherPart);
		mEditTextPart.setOnFocusChangeListener(new OnFocusChangeListener()
		{

			@Override
			public void onFocusChange(View v, boolean hasFocus)
			{
				// TODO Auto-generated method stub
				if (hasFocus)
				{
					SetKeyStates(mBasePart);
				}
			}
		});

		OnClickListener buttonListener = new OnClickListener()
		{

			@Override
			public void onClick(View v)
			{
				// TODO Auto-generated method stub
				if (mEventPending)
				{
					return;
				}

				mEventPending = true;

				Button button = (Button) v;
				CharSequence charSequence = button.getText();

				if (charSequence.charAt(0) == '0')
				{
					button.setText("1");
				}
				else
				{
					button.setText("0");
				}

				UpdateTextViewTotalText();
				UpdateTextViewPartText();

				mEventPending = false;
			}
		};

		LayoutParams layoutParams = new LinearLayout.LayoutParams(
				LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT, 1);

		for (int i = 1; i >= 0; i--)
		{
			LinearLayout linearLayoutIndex = new LinearLayout(this);
			LinearLayout linearLayoutValue = new LinearLayout(this);

			for (int j = 15; j >= 0; j--)
			{
				TextView textView = new TextView(this);
				textView.setText(String.format("%02d", 16 * i + j));
				textView.setLayoutParams(layoutParams);
				textView.setGravity(Gravity.CENTER);
				linearLayoutIndex.addView(textView);

				Button button = new Button(this);
				button.setText("0");
				button.setLayoutParams(layoutParams);
				button.setGravity(Gravity.CENTER);
				button.setOnClickListener(buttonListener);
				button.setOnClickListener(buttonListener);
				linearLayoutValue.addView(button);

				mButtonBits[16 * i + j] = button;
			}

			layoutRoot.addView(linearLayoutIndex);
			layoutRoot.addView(linearLayoutValue);
		}

		String stringKeys[] =
		{
			"0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
			"A", "B", "C", "D", "E", "F"
		};

		mButtonKeys = new Button[stringKeys.length];
		final int lines = 2;
		final int keys_per_line = stringKeys.length / lines;

		buttonListener = new OnClickListener()
		{

			@Override
			public void onClick(View v)
			{
				// TODO Auto-generated method stub
				View currView = getCurrentFocus();

				if (currView instanceof EditText)
				{
					Button button = (Button) v;
					EditText editText = (EditText) currView;

					editText.getText().insert(editText.getSelectionStart(), button.getText());
				}
			}
		};

		for (int i = 0; i < lines; i++)
		{
			LinearLayout linearLayout = new LinearLayout(this);
			final int start = i * keys_per_line;
			final int end = start + keys_per_line;

			for (int j = start; j < end; j++)
			{
				Button button = new Button(this);
				button.setText(stringKeys[j]);
				button.setLayoutParams(layoutParams);
				button.setGravity(Gravity.CENTER);
				button.setOnClickListener(buttonListener);
				linearLayout.addView(button);

				mButtonKeys[j] = button;
			}

			layoutRoot.addView(linearLayout);
		}

		BaseAdapter adapter;

		adapter = new BaseAdapter()
		{

			@Override
			public View getView(int position, View convertView, ViewGroup parent)
			{
				// TODO Auto-generated method stub
				TextView textView = new TextView(CalculatorActivity.this);

				textView.setText(String.format("%02d", position));
				textView.setTextColor(Color.BLACK);

				return textView;
			}

			@Override
			public long getItemId(int position)
			{
				// TODO Auto-generated method stub
				return 0;
			}

			@Override
			public Object getItem(int position)
			{
				// TODO Auto-generated method stub
				return null;
			}

			@Override
			public int getCount()
			{
				// TODO Auto-generated method stub
				return 32;
			}
		};

		mSpinnerStart = (Spinner) findViewById(R.id.spinner3);
		mSpinnerStart.setAdapter(adapter);
		mSpinnerStart.setSelection(mStart);
		mSpinnerStart.setOnItemSelectedListener(new OnItemSelectedListener()
		{

			@Override
			public void onItemSelected(AdapterView<?> arg0, View arg1,
					int arg2, long arg3)
			{
				// TODO Auto-generated method stub
				if (mEventPending)
				{
					return;
				}

				mEventPending = true;

				int start = mSpinnerStart.getSelectedItemPosition();
				int end = mStart + mLength - 1;

				if (start > end)
				{
					mStart = end;
					mSpinnerStart.setSelection(mStart);
				}
				else
				{
					mStart = start;
				}

				mLength = end - mStart + 1;
				mSpinnerLength.setSelection(mLength - 1);

				UpdateTextViewPartText();

				mEventPending = false;
			}

			@Override
			public void onNothingSelected(AdapterView<?> arg0)
			{
				// TODO Auto-generated method stub
				mSpinnerStart.setSelection(mStart);
			}
		});

		mSpinnerEnd = (Spinner) findViewById(R.id.spinner4);
		mSpinnerEnd.setAdapter(adapter);
		mSpinnerEnd.setSelection(mStart + mLength - 1);
		mSpinnerEnd.setOnItemSelectedListener(new OnItemSelectedListener()
		{

			@Override
			public void onItemSelected(AdapterView<?> arg0, View arg1,
					int arg2, long arg3)
			{
				// TODO Auto-generated method stub
				if (mEventPending)
				{
					return;
				}

				mEventPending = true;

				int end = mSpinnerEnd.getSelectedItemPosition();

				if (mStart < end)
				{
					mLength = end - mStart + 1;
				}
				else
				{
					mLength = 1;
					mSpinnerEnd.setSelection(mStart);
				}

				mSpinnerLength.setSelection(mLength - 1);

				UpdateTextViewPartText();

				mEventPending = false;
			}

			@Override
			public void onNothingSelected(AdapterView<?> arg0)
			{
				// TODO Auto-generated method stub
				mSpinnerEnd.setSelection(mStart + mLength - 1);
			}
		});

		adapter = new BaseAdapter()
		{
			@Override
			public View getView(int position, View convertView, ViewGroup parent)
			{
				// TODO Auto-generated method stub
				TextView textView = new TextView(CalculatorActivity.this);

				textView.setText(String.format("%02d", position + 1));
				textView.setTextColor(Color.BLACK);

				return textView;
			}

			@Override
			public long getItemId(int position)
			{
				// TODO Auto-generated method stub
				return 0;
			}

			@Override
			public Object getItem(int position)
			{
				// TODO Auto-generated method stub
				return null;
			}

			@Override
			public int getCount()
			{
				// TODO Auto-generated method stub
				return 32;
			}
		};

		mSpinnerLength = (Spinner) findViewById(R.id.spinner5);
		mSpinnerLength.setAdapter(adapter);
		mSpinnerLength.setSelection(mLength - 1);
		mSpinnerLength.setOnItemSelectedListener(new OnItemSelectedListener()
		{

			@Override
			public void onItemSelected(AdapterView<?> arg0, View arg1,
					int arg2, long arg3)
			{
				// TODO Auto-generated method stub
				if (mEventPending)
				{
					return;
				}

				mEventPending = true;

				int length = mSpinnerLength.getSelectedItemPosition() + 1;
				int max_length = 32 - mStart;

				if (length > max_length)
				{
					mLength = max_length;
					mSpinnerLength.setSelection(mLength - 1);
				}
				else
				{
					mLength = length;
				}

				mSpinnerEnd.setSelection(mStart + mLength - 1);

				UpdateTextViewPartText();

				mEventPending = false;
			}

			@Override
			public void onNothingSelected(AdapterView<?> arg0)
			{
				// TODO Auto-generated method stub
				mSpinnerLength.setSelection(mLength - 1);
			}
		});

		adapter = new BaseAdapter()
		{
			@Override
			public View getView(int position, View convertView, ViewGroup parent)
			{
				// TODO Auto-generated method stub
				TextView textView = new TextView(CalculatorActivity.this);

				textView.setText(String.format("%02d", position + 2));
				textView.setTextColor(Color.BLACK);

				return textView;
			}

			@Override
			public long getItemId(int position)
			{
				// TODO Auto-generated method stub
				return 0;
			}

			@Override
			public Object getItem(int position)
			{
				// TODO Auto-generated method stub
				return null;
			}

			@Override
			public int getCount()
			{
				// TODO Auto-generated method stub
				return 34;
			}
		};

		mSpinnerBaseTotal = (Spinner) findViewById(R.id.spinner1);
		mSpinnerBaseTotal.setAdapter(adapter);
		mSpinnerBaseTotal.setSelection(mBaseTotal - 2);
		mSpinnerBaseTotal.setOnItemSelectedListener(new OnItemSelectedListener()
		{

			@Override
			public void onItemSelected(AdapterView<?> arg0, View arg1,
					int arg2, long arg3)
			{
				// TODO Auto-generated method stub
				if (mEventPending)
				{
					return;
				}

				mEventPending = true;

				mBaseTotal = mSpinnerBaseTotal.getSelectedItemPosition() + 2;
				UpdateTextViewTotalText();
				EditText editText = (EditText) getCurrentFocus();
				if (editText == mEditTextTotal)
				{
					SetKeyStates(mBaseTotal);
				}

				mEventPending = false;
			}

			@Override
			public void onNothingSelected(AdapterView<?> arg0)
			{
				// TODO Auto-generated method stub
				mSpinnerBaseTotal.setSelection(mBaseTotal - 2);
			}
		});

		mSpinnerBasePart = (Spinner) findViewById(R.id.spinner2);
		mSpinnerBasePart.setAdapter(adapter);
		mSpinnerBasePart.setSelection(mBasePart - 2);
		mSpinnerBasePart.setOnItemSelectedListener(new OnItemSelectedListener()
		{

			@Override
			public void onItemSelected(AdapterView<?> arg0, View arg1,
					int arg2, long arg3)
			{
				// TODO Auto-generated method stub
				if (mEventPending)
				{
					return;
				}

				mEventPending = true;

				mBasePart = mSpinnerBasePart.getSelectedItemPosition() + 2;
				UpdateTextViewPartText();
				EditText editText = (EditText) getCurrentFocus();
				if (editText == mEditTextPart)
				{
					SetKeyStates(mBasePart);
				}

				mEventPending = false;
			}

			@Override
			public void onNothingSelected(AdapterView<?> arg0)
			{
				// TODO Auto-generated method stub
				mSpinnerBasePart.setSelection(mBasePart - 2);
			}
		});

		mEditTextShift = (EditText) findViewById(R.id.editText3);
		mEditTextShift.setOnFocusChangeListener(new OnFocusChangeListener()
		{

			@Override
			public void onFocusChange(View v, boolean hasFocus)
			{
				// TODO Auto-generated method stub
				if (hasFocus)
				{
					SetKeyStates(10);
				}
			}
		});
		mEditTextShift.setSelection(mEditTextShift.length());

		mButtonLeft = (Button) findViewById(R.id.button1);
		mButtonLeft.setOnClickListener(new OnClickListener()
		{

			@Override
			public void onClick(View v)
			{
				// TODO Auto-generated method stub
				long shift = StringToValue(mEditTextShift.getText().toString(), 10);

				UpdateTextViewTotalText(mTotalValue << shift);
			}
		});

		mButtonRight = (Button) findViewById(R.id.button2);
		mButtonRight.setOnClickListener(new OnClickListener()
		{

			@Override
			public void onClick(View v)
			{
				// TODO Auto-generated method stub
				long shift = StringToValue(mEditTextShift.getText().toString(), 10);

				UpdateTextViewTotalText((mTotalValue >> shift) & 0x7FFFFFFF);
			}
		});

		mButtonDel = (Button) findViewById(R.id.button3);
		mButtonDel.setOnClickListener(new OnClickListener()
		{

			@Override
			public void onClick(View v)
			{
				// TODO Auto-generated method stub
				View currView = getCurrentFocus();

				if (currView instanceof EditText)
				{
					EditText editText = (EditText) currView;
					int select = editText.getSelectionEnd();

					if (select > 0)
					{
						editText.getText().delete(select - 1, select);
					}
				}
			}
		});

		mButtonClr = (Button) findViewById(R.id.button4);
		mButtonClr.setOnClickListener(new OnClickListener()
		{

			@Override
			public void onClick(View v)
			{
				// TODO Auto-generated method stub
				View currView = getCurrentFocus();

				if (currView instanceof EditText)
				{
					EditText editText = (EditText) currView;

					editText.setText("");
				}
			}
		});

		UpdateTextViewTotalText();
		UpdateTextViewPartText();

		SetKeyStates(mBaseTotal);
	}
}
