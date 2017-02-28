package com.cavan.resource;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import android.app.AlertDialog.Builder;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.preference.DialogPreference;
import android.preference.PreferenceManager;
import android.text.Editable;
import android.util.ArraySet;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.ListView;

import com.cavan.android.CavanCheckBox;

public class EditableMultiSelectListPreference extends DialogPreference implements OnClickListener, OnCheckedChangeListener {

	public class Entry implements OnCheckedChangeListener {

		private String mKeyword;
		private boolean mEnabled;

		public Entry(String keyword, boolean enable) {
			mKeyword = keyword;
			mEnabled = enable;
		}

		public Entry(String text) {
			mKeyword = text.substring(0, text.length() - 2);
			mEnabled = text.charAt(text.length() - 1) != '0';
		}

		public String getKeyword() {
			return mKeyword;
		}

		public void setKeyword(String keyword) {
			mKeyword = keyword;
		}

		public boolean isEnabled() {
			return mEnabled;
		}

		public void setEnable(boolean enable) {
			mEnabled = enable;
		}

		@Override
		public String toString() {
			return mKeyword + ":" + (mEnabled ? '1' : '0');
		}

		@Override
		public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
			mEnabled = isChecked;

			if (isChecked) {
				isChecked = isAllEnabled();
			}

			mCheckBoxSelectAll.setCheckedSilent(isChecked);
		}
	}

	private Button mButtonAdd;
	private Button mButtonRemove;
	private EditText mEditTextKeyword;
	private ListView mListViewKeywords;
	private CavanCheckBox mCheckBoxSelectAll;

	private ArrayList<Entry> mEntries = new ArrayList<Entry>();
	private BaseAdapter mAdapter = new BaseAdapter() {

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			CavanCheckBox view;

			if (convertView != null) {
				view = (CavanCheckBox) convertView;
			} else {
				view = new CavanCheckBox(getContext());
			}

			Entry entry = mEntries.get(position);
			view.setText(entry.getKeyword());
			view.setCheckedSilent(entry.isEnabled());
			view.setOnCheckedChangeListener(entry);
			return view;
		}

		@Override
		public long getItemId(int position) {
			return position;
		}

		@Override
		public Object getItem(int position) {
			return mEntries.get(position);
		}

		@Override
		public int getCount() {
			return mEntries.size();
		}
	};

	public EditableMultiSelectListPreference(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
		super(context, attrs, defStyleAttr, defStyleRes);
	}

	public EditableMultiSelectListPreference(Context context, AttributeSet attrs, int defStyleAttr) {
		super(context, attrs, defStyleAttr);
	}

	public EditableMultiSelectListPreference(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public EditableMultiSelectListPreference(Context context) {
		super(context);
	}

	private boolean load() {
		mEntries.clear();

		String key = getKey();
		if (key == null || key.isEmpty()) {
			return false;
		}

		Set<String> values = getSharedPreferences().getStringSet(key, null);
		if (values == null) {
			return false;
		}

		boolean selectAll = true;

		for (String value : values) {
			if (value.length() < 3) {
				continue;
			}

			Entry entry = new Entry(value);
			mEntries.add(entry);

			if (!entry.isEnabled()) {
				selectAll = false;
			}
		}

		mCheckBoxSelectAll.setCheckedSilent(selectAll);

		return true;
	}

	public static List<String> load(Context context, String key) {
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
		if (preferences == null) {
			return null;
		}

		Set<String> values = preferences.getStringSet(key, null);
		if (values == null) {
			return null;
		}

		List<String> list = new ArrayList<String>();

		for (String value : values) {
			if (value.length() < 3) {
				continue;
			}

			if (value.charAt(value.length() - 1) == '0') {
				continue;
			}

			list.add(value.substring(0, value.length() - 2));
		}

		return list;
	}

	private boolean save() {
		String key = getKey();
		if (key == null || key.isEmpty()) {
			return false;
		}

		Set<String> values = new ArraySet<String>();
		for (Entry entry : mEntries) {
			values.add(entry.toString());
		}

		Editor editor = getEditor();
		editor.putStringSet(key, values);
		return editor.commit();
	}

	private boolean add() {
		Editable text = mEditTextKeyword.getText();
		if (text != null && text.length() > 0) {
			mEntries.add(new Entry(text.toString(), true));
			mAdapter.notifyDataSetChanged();
			text.clear();
			return true;
		}

		return false;
	}

	private int remove() {
		int count = 0;

		Iterator<Entry> iterator = mEntries.iterator();
		while (iterator.hasNext()) {
			Entry entry = iterator.next();
			if (entry.isEnabled()) {
				iterator.remove();
				count++;
			}
		}

		if (count > 0) {
			mAdapter.notifyDataSetChanged();
		}

		return count;
	}

	private boolean isAllEnabled() {
		for (Entry entry : mEntries) {
			if (!entry.isEnabled()) {
				return false;
			}
		}

		return true;
	}

	@Override
	protected void onPrepareDialogBuilder(Builder builder) {
		View view = LayoutInflater.from(getContext()).inflate(R.layout.editable_multi_select_list_preference, null);

		mButtonAdd = (Button) view.findViewById(R.id.buttonAdd);
		mButtonAdd.setOnClickListener(this);

		mButtonRemove = (Button) view.findViewById(R.id.buttonRemove);
		mButtonRemove.setOnClickListener(this);

		mEditTextKeyword = (EditText) view.findViewById(R.id.editTextKeyword);
		mListViewKeywords = (ListView) view.findViewById(R.id.listViewKeywords);
		mListViewKeywords.setAdapter(mAdapter);

		mCheckBoxSelectAll = (CavanCheckBox) view.findViewById(R.id.checkBoxSelectAll);
		mCheckBoxSelectAll.setOnCheckedChangeListener(this);

		load();

		builder.setView(view);
	}

	@Override
	protected void onDialogClosed(boolean positiveResult) {
		super.onDialogClosed(positiveResult);

		if (positiveResult) {
			add();
			save();
		}
	}

	@Override
	public void onClick(View v) {
		if (v == mButtonAdd) {
			add();
		} else if (v == mButtonRemove) {
			remove();
		}
	}

	@Override
	public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
		for (Entry entry : mEntries) {
			entry.setEnable(isChecked);
		}

		mAdapter.notifyDataSetChanged();
	}
}
