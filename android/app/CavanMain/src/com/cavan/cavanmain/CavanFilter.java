package com.cavan.cavanmain;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.database.Cursor;
import android.net.Uri;

import com.cavan.android.CavanDatabaseProvider;
import com.cavan.android.CavanDatabaseProvider.CavanDatabaseTable;

public class CavanFilter {

	private static final String TABLE_NAME = "filter";
	public static final Uri CONTENT_URI = Uri.withAppendedPath(CavanNotificationProvider.CONTENT_URI, TABLE_NAME);

	public static final String KEY_CONTENT = "content";
	public static final String KEY_ENABLE = "enable";

	public static final String[] PROJECTION = {
		CavanDatabaseProvider.KEY_ID, KEY_CONTENT, KEY_ENABLE
	};

	private long mIdentify;
	private String mContent;
	private boolean mEnable;

	public static void initDatabaseTable(CavanNotificationProvider provider) {
		CavanDatabaseTable table = provider.getTable(TABLE_NAME);

		table.setColumn(KEY_CONTENT, "text unique on conflict replace");
		table.setColumn(KEY_ENABLE, "integer");
	}

	public static Cursor query(ContentResolver resolver, String selection, String[] selectionArgs, String sortOrder) {
		return resolver.query(CONTENT_URI, PROJECTION, selection, selectionArgs, sortOrder);
	}

	public static Cursor query(ContentResolver resolver, String sortOrder) {
		return query(resolver, null, null, sortOrder);
	}

	public static Cursor query(ContentResolver resolver) {
		return query(resolver, null);
	}

	public static CavanFilter[] queryFilter(ContentResolver resolver, String selection, String[] selectionArgs, String sortOrder) {
		Cursor cursor = query(resolver, selection, selectionArgs, sortOrder);
		if (cursor == null) {
			return null;
		}

		CavanFilter[] filters = new CavanFilter[cursor.getCount()];
		for (int i = filters.length - 1; i >= 0; i--) {
			if (cursor.moveToPosition(i)) {
				filters[i] = new CavanFilter(cursor);
			}
		}

		return filters;
	}

	public static CavanFilter[] queryFilter(ContentResolver resolver, String sortOrder) {
		return queryFilter(resolver, null, null, sortOrder);
	}

	public static CavanFilter[] queryFilter(ContentResolver resolver) {
		return queryFilter(resolver, null);
	}

	public static CavanFilter[] queryFilterEnabled(ContentResolver resolver) {
		return queryFilter(resolver, KEY_ENABLE + "<>0", null, null);
	}

	public static int delete(ContentResolver resolver, String where, String[] selectionArgs) {
		return resolver.delete(CONTENT_URI, where, selectionArgs);
	}

	public static int deleteEnabled(ContentResolver resolver) {
		return delete(resolver, KEY_ENABLE + "<>0", null);
	}

	public CavanFilter(String content, boolean enable) {
		mIdentify = -1;
		mContent = content;
		mEnable = enable;
	}

	public CavanFilter(Cursor cursor) {
		parse(cursor);
	}

	public void parse(Cursor cursor) {
		mIdentify = cursor.getLong(0);
		mContent = cursor.getString(1);
		mEnable = cursor.getInt(2) != 0;
	}

	public long getIdentify() {
		return mIdentify;
	}

	public String getContent() {
		return mContent;
	}

	public void setContent(ContentResolver resolver, String content) {
		mContent = content;
		update(resolver);
	}

	public boolean isEnabled() {
		return mEnable;
	}

	public void setEnable(ContentResolver resolver, boolean enable) {
		mEnable = enable;
		update(resolver);
	}

	public Uri getIdentifyUri() {
		return Uri.withAppendedPath(CONTENT_URI, Long.toString(mIdentify));
	}

	public ContentValues getContentValues() {
		ContentValues values = new ContentValues();

		values.put(KEY_CONTENT, mContent);
		values.put(KEY_ENABLE, mEnable);

		return values;
	}

	public int update(ContentResolver resolver) {
		ContentValues values = getContentValues();

		if (mIdentify < 0) {
			resolver.insert(CONTENT_URI, values);
			return 1;
		} else {
			return resolver.update(getIdentifyUri(), values, null, null);
		}
	}

	public int delete(ContentResolver resolver) {
		return resolver.delete(getIdentifyUri(), null, null);
	}
}
