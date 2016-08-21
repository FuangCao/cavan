package com.cavan.cavanmain;

import java.util.regex.Pattern;

import android.database.Cursor;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ListView;

public class CavanMessageAdapter extends BaseAdapter {

	private ListView mView;
	private CavanMessageActivity mActivity;

	private boolean mFilterEnable;
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

	public void setFilterEnable(boolean enable) {
		mFilterEnable = enable;
	}

	public boolean isFilterEnabled() {
		return mFilterEnable;
	}

	public Cursor updateData() {
		String[] selectionArgs = null;
		String selection = null;

		mFilterPatterns = null;

		if (mFilterEnable) {
			CavanFilter[] filters = CavanFilter.queryFilterEnabled(mActivity.getContentResolver());
			if (filters != null && filters.length > 0) {
				mFilterPatterns = new Pattern[filters.length];
				StringBuilder builder = new StringBuilder();

				selectionArgs = new String[filters.length];

				for (int i = 0; i < filters.length; i++) {
					String text = filters[i].getContent();

					mFilterPatterns[i] = Pattern.compile(text, Pattern.CASE_INSENSITIVE);

					if (i > 0) {
						builder.append(" or ");
					}

					builder.append(CavanNotification.KEY_CONTENT + " like ?");
					selectionArgs[i] = "%" + text + "%";
				}

				builder.append(" collate nocase");

				selection = builder.toString();
			}
		}

		Cursor cursor = CavanNotification.query(mActivity.getContentResolver(), selection, selectionArgs, CavanNotification.KEY_TIMESTAMP);
		setCursor(cursor);

		return cursor;
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
