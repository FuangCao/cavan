package com.cavan.resource;

import java.net.InetSocketAddress;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Set;

import android.app.AlertDialog.Builder;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.res.TypedArray;
import android.preference.DialogPreference;
import android.preference.PreferenceManager;
import android.text.Editable;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.PopupMenu;
import android.widget.PopupMenu.OnMenuItemClickListener;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanCheckBox;
import com.cavan.java.CavanJava;
import com.cavan.java.CavanLinkedList;

public class EditableMultiSelectListPreference extends DialogPreference implements OnClickListener, OnCheckedChangeListener {

	public class Entry implements OnCheckedChangeListener, OnClickListener, OnMenuItemClickListener {

		private String mKeyword;
		private boolean mEnabled;

		public Entry(String keyword, boolean enable) {
			mKeyword = keyword;
			mEnabled = enable;
		}

		public Entry(String text) {
			if (text.length() > 0 && text.charAt(0) == '!') {
				mKeyword = text.substring(1);
				mEnabled = false;
			} else {
				mKeyword = text;
				mEnabled = true;
			}
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
			if (mEnabled) {
				return mKeyword;
			}

			return '!' + mKeyword;
		}

		@Override
		public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
			mEnabled = isChecked;

			if (isChecked) {
				isChecked = isAllEnabled();
			}

			mCheckBoxSelectAll.setCheckedSilent(isChecked);
		}

		@Override
		public void onClick(View v) {
			PopupMenu menu = new PopupMenu(v.getContext(), v);
			menu.inflate(R.menu.editable_checkbox);
			menu.setOnMenuItemClickListener(this);
			menu.show();
		}

		@Override
		public boolean onMenuItemClick(MenuItem item) {
			int id = item.getItemId();

			if (id == R.id.action_edit) {
				mEditTextKeyword.setText(mKeyword);
			} else if (id == R.id.action_move_up) {
				CavanLinkedList<Entry>.LinkNode node = mEntries.findNode(this);
				if (node != null && node.isNotFirstNode()) {
					node.shiftLeft();
					mAdapter.notifyDataSetChanged();
				}
			} else if (id == R.id.action_move_down) {
				CavanLinkedList<Entry>.LinkNode node = mEntries.findNode(this);
				if (node != null && node.isNotLastNode()) {
					node.shiftRight();
					mAdapter.notifyDataSetChanged();
				}
			} else if (id == R.id.action_remove) {
				mEntries.remove(this);
				mAdapter.notifyDataSetChanged();
			} else {
				return false;
			}

			return true;
		}
	}

	private Button mButtonAdd;
	private Button mButtonRemove;
	private EditText mEditTextKeyword;
	private ListView mListViewKeywords;
	private CavanCheckBox mCheckBoxSelectAll;

	private CavanLinkedList<Entry> mEntries = new CavanLinkedList<Entry>();
	private BaseAdapter mAdapter = new BaseAdapter() {

		private Entry[] mEntryArray = new Entry[0];

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			View view;

			if (convertView != null) {
				view = convertView;
			} else {
				view = View.inflate(getContext(), R.layout.editable_checkbox, null);
			}

			Entry entry = mEntryArray[position];
			CavanCheckBox checkbox = (CavanCheckBox) view.findViewById(R.id.checkBoxValue);

			checkbox.setText(entry.getKeyword());
			checkbox.setCheckedSilent(entry.isEnabled());
			checkbox.setOnCheckedChangeListener(entry);

			ImageView more = (ImageView) view.findViewById(R.id.imageViewMore);

			if (CavanAndroid.SDK_VERSION < 11) {
				more.setVisibility(View.GONE);
			} else {
				more.setOnClickListener(entry);
			}

			more.setOnClickListener(entry);

			return view;
		}

		@Override
		public long getItemId(int position) {
			return position;
		}

		@Override
		public Object getItem(int position) {
			return mEntryArray[position];
		}

		@Override
		public int getCount() {
			return mEntryArray.length;
		}

		@Override
		public void notifyDataSetChanged() {
			mEntryArray = mEntries.toArray(mEntryArray);
			super.notifyDataSetChanged();
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

	private static String[] loadPrivate(SharedPreferences preferences, String key) {
		try {
			String value = preferences.getString(key, null);
			CavanAndroid.dLog("value = " + value);
			if (value != null) {
				String[] lines = value.split("\\s*\\n\\s*");
				if (lines.length != 1) {
					return lines;
				}

				return lines[0].split("\\s*\\|\\s*");
			}
		} catch (Exception e) {
			e.printStackTrace();
		}

		try {
			Set<String> values = preferences.getStringSet(key, null);
			if (values != null) {
				String[] lines = new String[values.size()];
				values.toArray(lines);
				return lines;
			}
		} catch (Exception e) {
			e.printStackTrace();
		}

		return null;
	}

	public static ArrayList<String> load(SharedPreferences preferences, String key) {
		ArrayList<String> list = new ArrayList<String>();
		String[] lines = loadPrivate(preferences, key);

		if (lines != null) {
			for (String line : lines) {
				if (line.length() > 0 && line.charAt(0) != '!') {
					list.add(line);
				}
			}
		}

		return list;
	}

	public static ArrayList<String> load(Context context, String key) {
		return load(PreferenceManager.getDefaultSharedPreferences(context), key);
	}

	public static InetSocketAddress[] loadInetSocketAddresses(Context context, String key, int defPort) {
		ArrayList<String> lines = load(context, key);
		InetSocketAddress[] addresses = new InetSocketAddress[lines.size()];

		int i = 0;

		for (String line : lines) {
			int port;
			String[] args = line.split("\\s*:\\s*", 2);

			if (args.length > 1) {
				port = CavanJava.parseInt(args[1]);
			} else {
				port = defPort;
			}

			InetSocketAddress address = new InetSocketAddress(args[0], port);
			addresses[i++] = address;
		}

		return addresses;
	}

	private boolean addEntry(String keyword, boolean enabled) {
		int length = keyword.length();
		if (length > 0) {
			if (keyword.charAt(0) == '!') {
				if (length > 1) {
					keyword = keyword.substring(1);
					enabled = false;
				} else {
					return false;
				}
			}
		} else {
			return false;
		}

		for (Entry entry : mEntries) {
			if (entry.getKeyword().equals(keyword)) {
				return false;
			}
		}

		mEntries.add(new Entry(keyword, enabled));

		return true;
	}

	private boolean load() {
		String key = getKey();
		if (key == null || key.isEmpty()) {
			return false;
		}

		String[] lines = loadPrivate(getSharedPreferences(), key);
		if (lines == null) {
			return false;
		}

		mEntries.clear();

		for (String line : lines) {
			if (line.length() > 0) {
				addEntry(line, line.charAt(0) != '!');
			}
		}

		mAdapter.notifyDataSetChanged();

		return true;
	}

	private boolean save(String value) {
		Editor editor = getEditor();

		editor.putString(getKey(), value);

		try {
			editor.apply();
		} catch (Exception e) {
			editor.commit();
		}

		return true;
	}

	private boolean save() {
		StringBuilder builder = new StringBuilder();

		for (Entry entry : mEntries) {
			if (builder.length() > 0) {
				builder.append('|');
			}

			builder.append(entry.toString());
		}

		return save(builder.toString());
	}

	private boolean add() {
		Editable text = mEditTextKeyword.getText();
		if (text != null && text.length() > 0 && addEntry(text.toString(), true)) {
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

		if (CavanAndroid.SDK_VERSION < 11) {
			mButtonRemove.setOnClickListener(this);
		} else {
			mButtonRemove.setVisibility(View.INVISIBLE);
		}

		mEditTextKeyword = (EditText) view.findViewById(R.id.editTextKeyword);
		mListViewKeywords = (ListView) view.findViewById(R.id.listViewKeywords);
		mListViewKeywords.setAdapter(mAdapter);

		mCheckBoxSelectAll = (CavanCheckBox) view.findViewById(R.id.checkBoxSelectAll);
		mCheckBoxSelectAll.setCheckedSilent(isAllEnabled());
		mCheckBoxSelectAll.setOnCheckedChangeListener(this);

		builder.setView(view);
	}

	@Override
	protected void onDialogClosed(boolean positiveResult) {
		if (positiveResult) {
			add();

			if (callChangeListener(mEntries)) {
				save();
			}
		} else {
			load();
		}
	}

	@Override
	protected Object onGetDefaultValue(TypedArray a, int index) {
		return a.getString(index);
	}

	@Override
	protected void onSetInitialValue(boolean restorePersistedValue, Object defaultValue) {
		CavanAndroid.dLog("onSetInitialValue: " + restorePersistedValue);

		if (restorePersistedValue) {
			load();
		} else {
			CavanAndroid.dLog("defaultValue = " + defaultValue);

			if (defaultValue != null) {
				String value = (String) defaultValue;

				save(value);

				for (String text : value.split("\\s*\\|\\s*")) {
					addEntry(text, true);
				}

				mAdapter.notifyDataSetChanged();
			}
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
