package com.cavan.cavanmain;

import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;

import android.database.Cursor;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ListView;

public class CavanMessageAdapter extends BaseAdapter {

	private ListView mView;
	private CavanMessageActivity mActivity;

	private String[] mFilterLines;
	private Pattern[] mFilterPatterns;

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

	public CavanMessageAdapter(CavanMessageActivity activity) {
		super();

		mActivity = activity;

		mView = (ListView) activity.findViewById(R.id.listViewMessage);
		mView.setAdapter(this);

		updateFilter();
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

		if (mFilterLines != null && mFilterLines.length > 0) {
			String[] selectionArgs = new String[mFilterLines.length];
			StringBuilder selection = new StringBuilder();

			for (int i = 0; i < mFilterLines.length; i++) {
				if (i > 0) {
					selection.append(" or ");
				}

				selection.append(CavanNotification.KEY_CONTENT + " like ?");
				selectionArgs[i] = "%" + mFilterLines[i] + "%";
			}

			selection.append(" collate nocase");

			cursor = CavanNotification.query(mActivity.getContentResolver(), selection.toString(), selectionArgs, CavanNotification.KEY_TIMESTAMP);
		} else {
			cursor = CavanNotification.queryAll(mActivity.getContentResolver(), CavanNotification.KEY_TIMESTAMP);
		}

		setCursor(cursor);

		return cursor;
	}

	public void setFilter(List<String> lines) {
		if (lines == null) {
			mFilterLines = null;
			mFilterPatterns = null;
		} else {
			int size = lines.size();

			mFilterLines = new String[size];
			mFilterPatterns = new Pattern[size];

			for (int i = size - 1; i >= 0; i--) {
				String line = lines.get(i).trim();

				mFilterLines[i] = line;
				mFilterPatterns[i] = Pattern.compile(line, Pattern.CASE_INSENSITIVE);
			}
		}

		updateData();
	}

	public void setFilter(CavanFilter[] filters) {
		List<String> lines;

		if (filters == null) {
			lines = null;
		} else {
			lines = new ArrayList<String>();

			for (int i = filters.length - 1; i >= 0; i--) {
				if (filters[i].isEnabled()) {
					lines.add(filters[i].getContent());
				}
			}
		}

		setFilter(lines);
	}

	public void updateFilter() {
		CavanFilter[] filters = CavanFilter.queryFilterEnabled(mActivity.getContentResolver());
		setFilter(filters);
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
			view = CavanMessageView.getInstance(mActivity);
		} else {
			view = (CavanMessageView) convertView;
		}

		view.setTitle(mNotification.buildTitle());
		view.setContent(mNotification.getContent(), mNotification.getPackageName(), mFilterPatterns);

		return view;
	}
}
