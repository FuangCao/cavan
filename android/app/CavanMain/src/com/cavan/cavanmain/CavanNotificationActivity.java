package com.cavan.cavanmain;

import java.util.Date;

import android.app.Activity;
import android.database.ContentObserver;
import android.database.Cursor;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;

public class CavanNotificationActivity extends Activity {

	private static final int MAX_MESSAGE_COUNT = 200;

	private static final String[] PROJECTION = {
		CavanNotification.KEY_TIMESTAMP,
		CavanNotification.KEY_TITLE,
		CavanNotification.KEY_USER_NAME,
		CavanNotification.KEY_GROUP_NAME,
		CavanNotification.KEY_CONTENT,
	};

	private ListView mMessageView;
	private ContentObserver mContentObserver;
	private MessageAdapter mAdapter = new MessageAdapter();

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.notification_activity);

		mMessageView = (ListView) findViewById(R.id.listViewMessage);
		mMessageView.setAdapter(mAdapter);
		mAdapter.updateCursor();

		mContentObserver = new MessageObserver(new Handler());
		getContentResolver().registerContentObserver(CavanNotification.CONTENT_URI, true, mContentObserver);
	}

	@Override
	protected void onDestroy() {
		getContentResolver().unregisterContentObserver(mContentObserver);
		super.onDestroy();
	}

	public class MessageObserver extends ContentObserver {

		public MessageObserver(Handler handler) {
			super(handler);
		}

		@Override
		public void onChange(boolean selfChange) {
			mAdapter.updateCursor();
		}
	}

	public class MessageAdapter extends BaseAdapter {

		private Cursor mCursor;

		public void setCursor(Cursor cursor) {
			int count;

			if (cursor != null) {
				count = cursor.getCount();
			} else {
				count = 0;
			}

			String title = getResources().getString(R.string.text_message_count);
			setTitle(title + count);

			mCursor = cursor;
			notifyDataSetChanged();
		}

		public void updateCursor() {
			Cursor cursor = CavanNotification.queryAll(getContentResolver(), PROJECTION, CavanNotification.KEY_TIMESTAMP + " desc");
			setCursor(cursor);
		}

		@Override
		public int getCount() {
			if (mCursor == null) {
				return 0;
			}

			int count = mCursor.getCount();
			if (count > MAX_MESSAGE_COUNT) {
				return MAX_MESSAGE_COUNT;
			}

			return count;
		}

		@Override
		public Object getItem(int arg0) {
			return mCursor.moveToPosition(arg0);
		}

		@Override
		public long getItemId(int arg0) {
			return arg0;
		}

		@SuppressWarnings("deprecation")
		@Override
		public View getView(int arg0, View arg1, ViewGroup arg2) {
			if (!mCursor.moveToPosition(arg0)) {
				return null;
			}

			long time = mCursor.getLong(0);
			String title = mCursor.getString(1);
			String user = mCursor.getString(2);
			String group = mCursor.getString(3);
			String content = mCursor.getString(4);

			View view;

			if (arg1 != null) {
				view = arg1;
			} else {
				view = View.inflate(CavanNotificationActivity.this, R.layout.message_item, null);
			}

			TextView viewDate = (TextView) view.findViewById(R.id.textViewDate);
			TextView viewUser = (TextView) view.findViewById(R.id.textViewUser);
			TextView viewContent = (TextView) view.findViewById(R.id.textViewContent);

			if (user != null) {
				if (group != null) {
					user += "@" + group;
				}
			} else if (group != null) {
				user = group;
			} else if (title != null) {
				user = title;
			} else {
				user = "未知";
			}

			viewDate.setText(new Date(time).toLocaleString());
			viewDate.setBackgroundColor(Color.WHITE);
			viewDate.setTextColor(Color.BLACK);

			viewUser.setText(user);
			viewUser.setBackgroundColor(Color.WHITE);
			viewUser.setTextColor(Color.BLACK);

			viewContent.setText(content);
			viewContent.setTextColor(Color.BLACK);

			return view;
		}
	}
}
