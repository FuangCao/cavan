package com.cavan.cavanmain;

import java.util.ArrayList;
import java.util.List;

import android.database.Cursor;
import android.net.Uri;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ListView;

public class CavanMessageAdapter extends BaseAdapter {

	private ListView mView;
	private boolean mSelectionBottom;
	private MessageActivity mActivity;

	private int mCount;
	private Cursor mCursor;
	private Cursor mCursorPending;
	private List<CavanNotification> mNotifications = new ArrayList<CavanNotification>();

	private Runnable mRunnableUpdate = new Runnable() {

		@Override
		public void run() {
			int count;
			boolean isBottom = isSelectionBottom();

			if (mCursorPending != null) {
				mCursor = mCursorPending;
				mCursorPending = null;

				mNotifications.clear();

				count = mCursor.getCount();
			} else {
				count = mNotifications.size();

				if (mCursor != null) {
					count += mCursor.getCount();
				}
			}

			mCount = count;
			notifyDataSetChanged();

			if (isBottom) {
				mView.setSelection(count - 1);
				mSelectionBottom = false;
			}

			String title = mActivity.getResources().getString(R.string.message_count, count);
			mActivity.setTitle(title);
		}
	};

	public CavanMessageAdapter(MessageActivity activity) {
		super();

		mActivity = activity;

		mView = (ListView) activity.findViewById(R.id.listViewMessage);
		mView.setAdapter(this);
	}

	public void setSelectionBottom() {
		mSelectionBottom = true;
	}

	public boolean isSelectionBottom() {
		if (mSelectionBottom) {
			return true;
		}

		int childs = mView.getChildCount();
		if (childs > 0) {
			int position = mView.getLastVisiblePosition();
			if (position + 1 < mView.getCount()) {
				return false;
			} else {
				View last = mView.getChildAt(childs - 1);
				if (last.getBottom() > mView.getBottom()) {
					return false;
				}
			}
		}

		return true;
	}

	public boolean updateData(Uri uri, String selection, String[] selectionArgs, boolean bottom) {
		if (uri == null) {
			mCursorPending = CavanNotification.query(mActivity.getContentResolver(), selection, selectionArgs, null);
		} else {
			Cursor cursor = CavanNotification.query(mActivity.getContentResolver(), uri, selection, selectionArgs, null);
			if (cursor.moveToFirst()) {
				try {
					mNotifications.add(new CavanNotification(cursor));
				} catch (Exception e) {
					e.printStackTrace();
					return false;
				}
			} else {
				return false;
			}
		}

		mSelectionBottom = bottom;
		mView.post(mRunnableUpdate);

		return true;
	}

	@Override
	public int getCount() {
		return mCount;
	}

	@Override
	public Object getItem(int position) {
		return null;
	}

	@Override
	public long getItemId(int position) {
		return position;
	}

	@Override
	public View getView(int position, View convertView, ViewGroup viewGroup) {
		CavanNotification notification;

		try {
			if (mCursor.moveToPosition(position)) {
				notification = new CavanNotification(mCursor);
			} else {
				notification = mNotifications.get(position - mCursor.getCount());
			}
		} catch (Exception e) {
			e.printStackTrace();
			return null;
		}

		CavanMessageView view;

		if (convertView == null) {
			view = CavanMessageView.getInstance(mActivity);
		} else {
			view = (CavanMessageView) convertView;
		}

		view.setTitle(notification.buildTitle());

		String content = notification.getContent();
		if (content != null) {
			view.setContent(content, notification.getPackageName(), mActivity.getFilterPatterns());
		}

		return view;
	}
}
