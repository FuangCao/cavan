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
	private CavanMessageActivity mActivity;

	private int mCount;
	private Cursor mCursor;
	private List<Cursor> mCursors = new ArrayList<Cursor>();

	private CavanNotification mNotification = new CavanNotification();
	private Runnable mRunnableUpdate = new Runnable() {

		@Override
		public void run() {
			boolean isBottom = isSelectionBottom();
			int count = mCursors.size();

			if (mCursor != null) {
				count += mCursor.getCount();
			}

			mCount = count;
			notifyDataSetChanged();

			if (isBottom) {
				mView.setSelection(count - 1);
			}

			String title = mActivity.getResources().getString(R.string.text_message_count);
			mActivity.setTitle(title + count);
		}
	};

	public CavanMessageAdapter(CavanMessageActivity activity) {
		super();

		mActivity = activity;

		mView = (ListView) activity.findViewById(R.id.listViewMessage);
		mView.setAdapter(this);
	}

	public boolean isSelectionBottom() {
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

	public void updateData(Uri uri, String selection, String[] selectionArgs) {
		if (uri == null) {
			mCursor = CavanNotification.query(mActivity.getContentResolver(), selection, selectionArgs, CavanNotification.KEY_TIMESTAMP);
			mCursors.clear();
		} else {
			Cursor cursor = CavanNotification.query(mActivity.getContentResolver(), uri, selection, selectionArgs, CavanNotification.KEY_TIMESTAMP);
			if (cursor.moveToFirst()) {
				mCursors.add(cursor);
			} else {
				return;
			}
		}

		mView.post(mRunnableUpdate);
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
		Cursor cursor;

		if (mCursor.moveToPosition(position)) {
			cursor = mCursor;
		} else {
			cursor = mCursors.get(position - mCursor.getCount());
		}

		if (!mNotification.parse(cursor)) {
			return null;
		}

		CavanMessageView view;

		if (convertView == null) {
			view = CavanMessageView.getInstance(mActivity);
		} else {
			view = (CavanMessageView) convertView;
		}

		view.setTitle(mNotification.buildTitle());

		String content = mNotification.getContent();
		if (content != null) {
			view.setContent(content, mNotification.getPackageName(), mActivity.getFilterPatterns());
		}

		return view;
	}
}
