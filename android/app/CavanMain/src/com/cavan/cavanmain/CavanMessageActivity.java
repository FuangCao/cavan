package com.cavan.cavanmain;

import java.util.List;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.FragmentManager;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.database.ContentObserver;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.EditText;
import android.widget.ListView;

import com.cavan.android.CavanAndroid;

public class CavanMessageActivity extends Activity {

	private ListView mMessageView;
	private CharSequence mFilterText;
	private CavanMessageAdapter mAdapter;
	private ContentObserver mContentObserver = new ContentObserver(new Handler()) {

		@Override
		public void onChange(boolean selfChange) {
			updateData();
		}
	};

	CavanMessageFinder mFinder = new CavanMessageFinder();

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

			mMessageView = (ListView) findViewById(R.id.listViewMessage);
			mAdapter = new CavanMessageAdapter(mMessageView);
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
			mFinder.show(getFragmentManager());
			break;
		}

		return super.onOptionsItemSelected(item);
	}

	public class CavanMessageFinder extends DialogFragment implements OnClickListener {

		private EditText mEditTextFilter;

		public void show(FragmentManager manager) {
			super.show(manager, CavanAndroid.TAG);
		}

		@Override
		public Dialog onCreateDialog(Bundle savedInstanceState) {
			View view = getLayoutInflater().inflate(R.layout.message_filter, null);
			mEditTextFilter = (EditText) view.findViewById(R.id.editTextFilter);
			mEditTextFilter.setText(mFilterText);

			AlertDialog.Builder builder = new AlertDialog.Builder(CavanMessageActivity.this);

			builder.setView(view);
			builder.setCancelable(false);
			builder.setPositiveButton(R.string.text_filter, this);
			builder.setNeutralButton(R.string.text_filter_none, this);
			builder.setNegativeButton(android.R.string.cancel, null);

			return builder.create();
		}

		@Override
		public void onClick(DialogInterface dialog, int which) {
			mFilterText = mEditTextFilter.getText();

			switch (which) {
			case DialogInterface.BUTTON_POSITIVE:
				mAdapter.setFilter(mFilterText.toString());
				break;

			case DialogInterface.BUTTON_NEUTRAL:
				mAdapter.setFilter(null);
				break;
			}

			updateData();
		}
	}
}
