package com.cavan.calculator2;

import android.content.Context;
import android.graphics.Color;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;

public class BitMapAdapter extends BaseAdapter implements OnClickListener
{
	@SuppressWarnings("unused")
	private static final String TAG = "BitMapAdapter";
	private static final int COLOR_ZERO = Color.GRAY;
	private static final int COLOR_ONE = Color.RED;

	private long mValue;
	private BitMapButton[] mButtons;
	private OnValueChangedListener mValueChangedListener;

	public BitMapAdapter(Context context, long value, int count)
	{
		mButtons = new BitMapButton[count];
		for (int i = mButtons.length - 1; i >= 0; i--)
		{
			mButtons[i] = new BitMapButton(context, i);
			mButtons[i].setOnClickListener(this);
		}

		setValue(value);
	}

	@Override
	public int getCount()
	{
		return mButtons.length;
	}

	@Override
	public Object getItem(int position)
	{
		return null;
	}

	@Override
	public long getItemId(int position)
	{
		return 0;
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent)
	{
		return mButtons[mButtons.length - position - 1];
	}

	@Override
	public void onClick(View v)
	{
		BitMapButton button = (BitMapButton) v;
		long mask = ((long)1) << button.getIndex();
		long oldValue = mValue;
		int newColor;

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
			button.setTextColor(newColor);
		}
		else
		{
			mValue = oldValue;
		}
	}

	void setValue(long value)
	{
		mValue = value;

		for (int i = mButtons.length - 1; i >= 0; i--)
		{
			if ((mValue & (((long)1) << i)) == 0)
			{
				mButtons[i].setTextColor(COLOR_ZERO);
			}
			else
			{
				mButtons[i].setTextColor(COLOR_ONE);
			}
		}
	}

	void setValue(String text, int base)
	{
		byte[] bytes = text.getBytes();
		long value = 0;

		for (int i = 0; i < bytes.length; i++)
		{
			value = value * base + ByteToValue(bytes[i]);
		}

		setValue(value);
	}

	void setOnValueChangedListener(OnValueChangedListener listener)
	{
		mValueChangedListener = listener;
	}

	public static long ByteToValue(byte b)
	{
		if (b >= '0' && b <= '9')
		{
			return b - '0';
		}
		else if (b >= 'a' && b <= 'z')
		{
			return b - 'a' + 10;
		}
		else if (b >= 'A' && b <= 'Z')
		{
			return b - 'A' + 10;
		}
		else
		{
			return -1;
		}
	}

	public static byte ValueToByte(long value)
	{
		if (value >= 0 && value <= 9)
		{
			return (byte) (value + '0');
		}

		return (byte) (value + 'A' - 10);
	}

	public static byte[] ByteArrayExchange(byte[] bytes, int index, int size)
	{
		for (int last = index + size - 1; index < last; index++, last--)
		{
			byte temp = bytes[index];
			bytes[index] = bytes[last];
			bytes[last] = temp;
		}

		return bytes;
	}

	String toString(int base, int size)
	{
		if (mValue == 0 && size == 0)
		{
			return "0";
		}

		byte[] bytes = new byte[128];
		long value;
		int i;

		for (value = mValue, i = 0; value != 0; value /= base, i++)
		{
			bytes[i] = ValueToByte(value % base);
		}

		for (size -= i; size > 0; size--, i++)
		{
			bytes[i] = '0';
		}

		return new String(ByteArrayExchange(bytes, 0, i), 0, i);
	}

	@Override
	public String toString()
	{
		return toString(10, 0);
	}
}

class BitMapButton extends Button
{
	private int mIndex;

	public BitMapButton(Context context, int index)
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