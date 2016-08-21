package com.cavan.cavanmain;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.database.Cursor;
import android.net.Uri;

import com.cavan.android.CavanDatabaseProvider.CavanDatabaseTable;

public class CavanSetting {

	private static final String TABLE_NAME = "settings";
	public static final Uri CONTENT_URI = Uri.withAppendedPath(CavanNotificationProvider.CONTENT_URI, TABLE_NAME);

	public static final String KEY_NAME = "name";
	public static final String KEY_VALUE = "value";

	private static final String[] PROJECTION = {
		KEY_VALUE
	};

	private String mName;
	private String mValue;

	public CavanSetting(String key, String value) {
		mName = key;
		mValue = value;
	}

	public CavanSetting(Cursor cursor) {
		parse(cursor);
	}

	public void parse(Cursor cursor) {
		mName = cursor.getString(0);
		mValue = cursor.getString(1);
	}

	public String getName() {
		return mName;
	}

	public String getValue() {
		return mValue;
	}

	public void setValue(String value) {
		mValue = value;
	}

	public ContentValues getContentValues() {
		ContentValues values = new ContentValues();

		values.put(KEY_NAME, mName);
		values.put(KEY_VALUE, mValue);

		return values;
	}

	public static void initDatabaseTable(CavanNotificationProvider provider) {
		CavanDatabaseTable table = provider.getTable(TABLE_NAME);

		table.setColumn(KEY_NAME, "text");
		table.setColumn(KEY_VALUE, "text unique on conflict replace");
	}

	public static Uri put(ContentResolver resolver, String key, String value) {
		CavanSetting setting = new CavanSetting(key, value);
		return resolver.insert(CONTENT_URI, setting.getContentValues());
	}

	public static String get(ContentResolver resolver, String key) {
		String selection = KEY_NAME + "=?";
		String[] selectionArgs = { key };
		Cursor cursor = resolver.query(CONTENT_URI, PROJECTION, selection, selectionArgs, null);
		if (cursor == null || cursor.getCount() > 0) {
			return null;
		}

		if (!cursor.moveToFirst()) {
			return null;
		}

		return cursor.getString(0);
	}

	public static boolean getBoolean(ContentResolver resolver, String key) {
		String value = get(resolver, key);
		if (value == null) {
			return false;
		}

		return Boolean.valueOf(value);
	}
}
