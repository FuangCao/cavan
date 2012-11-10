package com.cavan.calculator2;

import android.content.Context;
import android.graphics.Color;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.GridView;

public class BitmapGridView extends GridView implements OnClickListener
{
	private static final String TAG = "BitmapGridView";

	private static final int COLOR_ZERO = Color.GRAY;
	private static final int COLOR_ONE = Color.CYAN;

	private long mValue;
	private BitmapButton[] mButtons = new BitmapButton[32];
	private OnValueChangedListener mValueChangedListener;

	private BaseAdapter mAdapter = new BaseAdapter()
	{
		public View getView(int position, View convertView, ViewGroup parent)
		{
			return mButtons[mButtons.length - position - 1];
		}

		public long getItemId(int position)
		{
			return 0;
		}

		public Object getItem(int arg0)
		{
			return null;
		}

		public int getCount()
		{
			return mButtons.length;
		}
	};

	public BitmapGridView(Context context, AttributeSet attrs)
	{
		super(context, attrs);

		for (int i = mButtons.length - 1; i >= 0; i--)
		{
			mButtons[i] = new BitmapButton(context, i);
			mButtons[i].setOnClickListener(this);
		}

		setValue(0);
		setNumColumns(8);
		setAdapter(mAdapter);
	}

	public long getValue()
	{
		return mValue;
	}

	public void setValue(long value)
	{
		mValue = value;
		for (int i = 0; i < 32; i++)
		{
			if ((mValue & (((long)1) << i)) == 0)
			{
				mButtons[i].setBackgroundColor(COLOR_ZERO);
			}
			else
			{
				mButtons[i].setBackgroundColor(COLOR_ONE);
			}
		}
	}

	public void setValue(String text, int base)
	{
		setValue(FormulaCalculator.TextToLong(text, base));
	}

	public void onClick(View v)
	{
		Log.i(TAG, "onClick");
		BitmapButton button = (BitmapButton) v;
		long oldValue = mValue;
		int newColor;
		long mask = ((long)1) << button.getIndex();
		if ((mValue & mask) == 0)
		{
			mValue |= mask;
			newColor = COLOR_ONE;
		}
		else
		{
			mValue &= (~mask);
			newColor = COLOR_ZERO;
		}

		if (mValueChangedListener == null || mValueChangedListener.OnValueChanged(oldValue, mValue))
		{
			button.setBackgroundColor(newColor);
		}
		else
		{
			mValue = oldValue;
		}
	}

	@Override
	public String toString()
	{
		return FormulaCalculator.LongToText(mValue, 10, 0);
	}

	public String toString(int base)
	{
		return FormulaCalculator.LongToText(mValue, base, 0);
	}

	public String toString(int base, int size)
	{
		return FormulaCalculator.LongToText(mValue, base, size);
	}

	public void setOnValueChangedListener(OnValueChangedListener onValueChangedListener)
	{
		mValueChangedListener = onValueChangedListener;
	}
}

class BitmapButton extends Button
{
	private int mIndex;

	public BitmapButton(Context context, int index)
	{
		super(context);
		mIndex = index;
		setText(String.format("%02d", mIndex));
	}

	public int getIndex()
	{
		return mIndex;
	}

	public void setIndex(int index)
	{
		this.mIndex = index;
	}
}

interface OnValueChangedListener
{
	boolean OnValueChanged(long oldValue, long newValue);
}