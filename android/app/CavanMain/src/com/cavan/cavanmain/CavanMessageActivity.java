package com.cavan.cavanmain;

import java.util.List;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.FragmentManager;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.database.ContentObserver;
import android.database.Cursor;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.text.Editable;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.ListView;

import com.cavan.android.CavanAndroid;

public class CavanMessageActivity extends Activity {

	public static final int FLAG_NEEDS_MENU_KEY = 0x40000000;

	private CavanMessageAdapter mAdapter;
	private CavanMessageFilter mMessageFinder = new CavanMessageFilter();
	private ContentObserver mContentObserver = new ContentObserver(new Handler()) {

		@Override
		public void onChange(boolean selfChange) {
			updateData();
		}
	};

	private void updateData() {
		Cursor cursor = mAdapter.updateData();
		if (cursor != null) {
			String title = getResources().getString(R.string.text_message_count);
			setTitle(title + cursor.getCount());
		}
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		Uri uri = getIntent().getData();
		if (uri != null) {
			CavanAndroid.eLog("uri = " + uri);

			try {
				List<String> paths = uri.getPathSegments();
				String action = paths.get(0);

				if (CavanMessageView.ACTION_ALIPAY.equals(action)) {
					String code = paths.get(1);

					RedPacketListenerService.postRedPacketCode(this, paths.get(1));
					RedPacketListenerService.startAlipayActivity(this);

					CavanAndroid.showToastLong(this, "支付宝口令: " + code);
				} else if (CavanMessageView.ACTION_OPEN.equals(action)) {
					Intent intent = getPackageManager().getLaunchIntentForPackage(uri.getHost());
					if (intent != null) {
						intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED);
						startActivity(intent);
					}
				}
			} catch (Exception e) {
				e.printStackTrace();
			}

			finish();
		} else {
			setContentView(R.layout.notification_activity);
			getWindow().setFlags(FLAG_NEEDS_MENU_KEY, FLAG_NEEDS_MENU_KEY);

			mAdapter = new CavanMessageAdapter(this);
			updateData();

			getContentResolver().registerContentObserver(CavanNotification.CONTENT_URI, true, mContentObserver);
		}
	}

	@Override
	protected void onDestroy() {
		if (mContentObserver != null) {
			getContentResolver().unregisterContentObserver(mContentObserver);
		}

		super.onDestroy();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.message_activity, menu);
		return super.onCreateOptionsMenu(menu);
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case R.id.action_message_clean:
			int count = CavanNotification.deleteAll(getContentResolver());
			CavanAndroid.showToast(this, String.format("成功清除 %d 条消息", count));
			updateData();
			break;

		case R.id.action_message_finder:
			mMessageFinder.show(getFragmentManager());
			break;
		}

		return super.onOptionsItemSelected(item);
	}

	public class CavanMessageFilter extends DialogFragment implements OnCheckedChangeListener, OnClickListener {

		private CavanFilter[] mFilters;

		private Button mButtonAdd;
		private Button mButtonDelete;
		private ListView mListViewFilter;
		private EditText mEditTextFilter;
		private CheckBox mCheckBoxSelectAll;

		private BaseAdapter mFilterAdapter = new BaseAdapter() {

			@Override
			public void notifyDataSetChanged() {
				mFilters = CavanFilter.queryFilter(getContentResolver());
				super.notifyDataSetChanged();
			}

			@Override
			public View getView(int position, View convertView, ViewGroup parent) {
				CavanFilterView view = (CavanFilterView) convertView;
				if (view == null) {
					view = new CavanFilterView(CavanMessageActivity.this, mFilters[position]);
				} else {
					view.setFilter(mFilters[position]);
				}

				return view;
			}

			@Override
			public long getItemId(int position) {
				return 0;
			}

			@Override
			public Object getItem(int position) {
				return null;
			}

			@Override
			public int getCount() {
				if (mFilters != null) {
					return mFilters.length;
				}

				return 0;
			}
		};

		public void show(FragmentManager manager) {
			super.show(manager, CavanAndroid.TAG);
		}

		@Override
		public Dialog onCreateDialog(Bundle savedInstanceState) {
			View view = getLayoutInflater().inflate(R.layout.message_filter, null);

			mCheckBoxSelectAll = (CheckBox) view.findViewById(R.id.checkBoxSelectAll);
			mCheckBoxSelectAll.setTextColor(Color.WHITE);
			mCheckBoxSelectAll.setOnCheckedChangeListener(this);

			mButtonAdd = (Button) view.findViewById(R.id.buttonAdd);
			mButtonAdd.setOnClickListener(this);

			mButtonDelete = (Button) view.findViewById(R.id.buttonDelete);
			mButtonDelete.setOnClickListener(this);

			mEditTextFilter = (EditText) view.findViewById(R.id.editTextFilter);

			mListViewFilter = (ListView) view.findViewById(R.id.listViewFilter);
			mListViewFilter.setAdapter(mFilterAdapter);
			mFilterAdapter.notifyDataSetChanged();

			AlertDialog.Builder builder = new AlertDialog.Builder(CavanMessageActivity.this);

			builder.setView(view);
			builder.setCancelable(false);
			builder.setPositiveButton(R.string.text_filter, null);

			return builder.create();
		}

		@Override
		public void onDismiss(DialogInterface dialog) {
			mAdapter.setFilter(mFilters);
			super.onDismiss(dialog);
		}

		@Override
		public void onClick(View v) {
			switch (v.getId()) {
			case R.id.buttonAdd:
				Editable editable = mEditTextFilter.getText();
				CavanFilter filter = new CavanFilter(editable.toString(), true);
				filter.update(getContentResolver());
				mFilterAdapter.notifyDataSetChanged();
				editable.clear();
				break;

			case R.id.buttonDelete:
				CavanFilter.deleteEnabled(getContentResolver());
				mFilterAdapter.notifyDataSetChanged();
				break;
			}
		}

		@Override
		public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
			if (mFilters == null) {
				return;
			}

			for (int i = mFilters.length - 1; i >= 0; i--) {
				mFilters[i].setEnable(getContentResolver(), isChecked);
			}

			mFilterAdapter.notifyDataSetChanged();
		}
	}

	public class CavanFilterView extends CheckBox implements OnCheckedChangeListener {

		private CavanFilter mFilter;

		public CavanFilterView(Context context, CavanFilter filter) {
			super(context);

			setTextColor(Color.WHITE);
			setFilter(filter);

			setOnCheckedChangeListener(this);
		}

		public void updateData() {
			setText(mFilter.getContent());
			setChecked(mFilter.isEnabled());
		}

		public void setFilter(CavanFilter filter) {
			mFilter = filter;
			updateData();
		}

		public void delete(ContentResolver resolver) {
			mFilter.delete(resolver);
		}

		@Override
		public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
			mFilter.setEnable(getContentResolver(), isChecked);
		}
	}
}
