package com.cavan.cavanmain;

import java.util.List;
import java.util.regex.Pattern;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.FragmentManager;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.database.ContentObserver;
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

import com.cavan.accessibility.CavanNotification;
import com.cavan.android.CavanAndroid;

public class MessageActivity extends Activity {

	private CavanMessageAdapter mAdapter;
	private CavanMessageFilter mMessageFinder = new CavanMessageFilter();
	private ContentObserver mContentObserverMessage = new ContentObserver(new Handler()) {

		@Override
		public void onChange(boolean selfChange, Uri uri) {
			updateData(uri, false);
		}
	};

	private boolean mFilterEnable;
	private String mSelection;
	private String[] mSelectionArgs;
	private Pattern[] mFilterPatterns;

	public static Intent getIntent(Context context) {
		return new Intent(context, MessageActivity.class);
	}

	public Pattern[] getFilterPatterns() {
		return mFilterPatterns;
	}

	public void updateData(Uri uri, boolean bottom) {
		mAdapter.updateData(uri, mSelection, mSelectionArgs, bottom);
	}

	public void updateFilter() {
		CavanFilter[] filters;

		if (mFilterEnable) {
			filters = CavanFilter.queryFilterEnabled(getContentResolver());
		} else {
			filters = null;
		}

		if (filters != null && filters.length > 0) {
			mFilterPatterns = new Pattern[filters.length];
			StringBuilder builder = new StringBuilder();

			mSelectionArgs = new String[filters.length];

			for (int i = 0; i < filters.length; i++) {
				String text = filters[i].getContent();

				mFilterPatterns[i] = Pattern.compile(text, Pattern.CASE_INSENSITIVE);

				if (i > 0) {
					builder.append(" or ");
				}

				builder.append(CavanNotification.KEY_CONTENT + " like ?");
				mSelectionArgs[i] = "%" + text + "%";
			}

			builder.append(" collate nocase");

			mSelection = builder.toString();
		} else {
			mSelection = null;
			mSelectionArgs = null;
			mFilterPatterns = null;
		}

		updateData(null, true);
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		mMessageFinder.setMessageActivity(this);

		Uri uri = getIntent().getData();
		if (uri != null) {
			CavanAndroid.dLog("uri = " + uri);

			try {
				List<String> paths = uri.getPathSegments();
				String action = paths.get(0);

				if (CavanMessageView.ACTION_ALIPAY.equals(action)) {
					String code = paths.get(1);

					RedPacketListenerService.postRedPacketCode(this, paths.get(1));
					RedPacketListenerService.startAlipayActivity(this);

					FloatMessageService.showToast("支付宝口令: " + code);
				} else if (CavanMessageView.ACTION_OPEN.equals(action)) {
					CavanAndroid.startActivity(this, uri.getHost());
				}
			} catch (Exception e) {
				e.printStackTrace();
			}

			finish();
		} else {
			setContentView(R.layout.notification_activity);
			CavanAndroid.setMenuKeyVisibility(getWindow(), true);

			mAdapter = new CavanMessageAdapter(this);
			updateData(null, true);

			getContentResolver().registerContentObserver(CavanNotificationTable.CONTENT_URI, true, mContentObserverMessage);
		}
	}

	@Override
	protected void onDestroy() {
		if (mContentObserverMessage != null) {
			getContentResolver().unregisterContentObserver(mContentObserverMessage);
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
			AlertDialog.Builder builder = new AlertDialog.Builder(this);
			builder.setMessage(R.string.clean_message_confirm);
			builder.setCancelable(true);
			builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {

				@Override
				public void onClick(DialogInterface dialog, int which) {
					int count = CavanNotificationTable.deleteAll(getContentResolver());
					CavanAndroid.showToast(getApplicationContext(), String.format("成功清除 %d 条消息", count));
					updateData(null, true);
				}
			});
			builder.setNegativeButton(android.R.string.cancel, null);
			builder.create().show();

			break;

		case R.id.action_message_finder:
			mMessageFinder.show(getFragmentManager());
			break;
		}

		return super.onOptionsItemSelected(item);
	}

	public void setFilterEnable(boolean enable) {
		mFilterEnable = enable;
	}
}
