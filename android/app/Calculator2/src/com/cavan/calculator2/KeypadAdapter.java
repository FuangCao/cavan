package com.cavan.calculator2;

import android.content.Context;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;

public class KeypadAdapter extends BaseAdapter implements OnClickListener
{
	private static String[] KEY_NAMES =
	{
		"Clean", "Back", "<<", ">>", "+", "-", "*", "/",
		"%", "&", "|", "^", "~", "!", "@",
		"0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
		"A", "B", "C", "D", "E", "F",
	};

	private Button[] mButtons;
	private OnKeypadChangedListener mKeypadChangedListener;

	public KeypadAdapter(Context context)
	{
		mButtons = new Button[KEY_NAMES.length];
		for (int i = 0; i < mButtons.length; i++)
		{
			mButtons[i] = new Button(context);
			mButtons[i].setText(KEY_NAMES[i]);
		}
	}

	public int getCount()
	{
		return mButtons.length;
	}

	public Object getItem(int position)
	{
		return null;
	}

	public long getItemId(int position)
	{
		return 0;
	}

	public View getView(int position, View convertView, ViewGroup parent)
	{
		return mButtons[position];
	}

	public void setOnKeypadChangedListener(OnKeypadChangedListener onKeypadChangedListener)
	{
		mKeypadChangedListener = onKeypadChangedListener;
		OnClickListener listener = mKeypadChangedListener == null ? null : this;
		for (int i = 0; i < mButtons.length; i++)
		{
			mButtons[i].setOnClickListener(listener);
		}
	}

	public void onClick(View v)
	{
		if (v.equals(mButtons[0]))
		{
			mKeypadChangedListener.OnCleanKeyClicked((Button) v);
		}
		else if (v.equals(mButtons[1]))
		{
			mKeypadChangedListener.OnBackKeyClicked((Button) v);
		}
		else
		{
			mKeypadChangedListener.OnTextKeyClicked((Button) v);
		}
	}
}

interface OnKeypadChangedListener
{
	void OnTextKeyClicked(Button button);
	void OnCleanKeyClicked(Button button);
	void OnBackKeyClicked(Button button);
}
