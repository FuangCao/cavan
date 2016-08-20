package com.cavan.cavanmain;

import java.util.regex.Pattern;

import android.content.Context;
import android.database.Cursor;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ListView;

public class CavanMessageAdapter extends BaseAdapter {

	private ListView mView;
	private Context mContext;

	private String mFilterText;
	private Pattern mFilterPattern;

	private Cursor mCursor;
	private Cursor mCursorPending;
	private CavanNotification mNotification = new CavanNotification();
	private Runnable mRunnableUpdate = new Runnable() {

		@Override
		public void run() {
			if (mCursorPending == null) {
				return;
			}

			int count;
			Cursor cursor = mCursorPending;

			mCursorPending = null;

			if (cursor != null) {
				count = cursor.getCount();
			} else {
				count = 0;
			}

			boolean isBottom = isSelectionBottom();

			mCursor = cursor;
			notifyDataSetChanged();

			if (isBottom) {
				mView.setSelection(count - 1);
			}
		}
	};

	public CavanMessageAdapter(ListView view) {
		super();

		mView = view;
		mContext = view.getContext();

		view.setAdapter(this);
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

	synchronized public void setCursor(Cursor cursor) {
		mCursorPending = cursor;
		mView.post(mRunnableUpdate);
	}

	public Cursor updateData() {
		Cursor cursor;

		if (mFilterText == null) {
			cursor = CavanNotification.queryAll(mContext.getContentResolver(), CavanNotification.KEY_TIMESTAMP);
		} else {
			String selection = CavanNotification.KEY_CONTENT + " like ? collate nocase";
			String[] selectionArgs = { "%" + mFilterText + "%" };
			cursor = CavanNotification.query(mContext.getContentResolver(), selection, selectionArgs, CavanNotification.KEY_TIMESTAMP);
		}

		setCursor(cursor);

		return cursor;
	}

	public void setFilter(String filter) {
		if (filter == null || filter.isEmpty()) {
			mFilterText = null;
			mFilterPattern = null;
		} else {
			mFilterText = filter;
			mFilterPattern = Pattern.compile(filter, Pattern.CASE_INSENSITIVE);
		}
	}

	public String getFilter() {
		return mFilterText;
	}

	@Override
	public int getCount() {
		if (mCursor == null) {
			return 0;
		}

		return mCursor.getCount();
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
		if (!mCursor.moveToPosition(position)) {
			return null;
		}

		if (!mNotification.parse(mCursor)) {
			return null;
		}

		CavanMessageView view;

		if (convertView == null) {
			view = CavanMessageView.getInstance(mContext);
		} else {
			view = (CavanMessageView) convertView;
		}

		view.setTitle(mNotification.buildTitle());
		view.setContent(mNotification.getContent(), mNotification.getPackageName(), mFilterPattern);

		return view;
	}
}
