package com.cavan.jwaootoyfactorytest;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanDatabaseProvider.CavanDatabaseTable;

public class TestResult {

	public static final String TABLE_NAME = "result";
	public static final Uri CONTENT_URI = Uri.withAppendedPath(DatabaseProvider.CONTENT_URI, TABLE_NAME);

	public static final String KEY_ADDRESS = "address";
	public static final String KEY_TIME = "time";
	public static final String KEY_VALUE = "value";
	public static final String KEY_MASK = "mask";

	public static final String[] PROJECTION = {
		KEY_TIME, KEY_VALUE, KEY_MASK
	};

	private String mAddress;
	private long mTime;
	private int mValue;
	private int mMask;

	public static void initDatabaseTable(DatabaseProvider provider) {
		CavanDatabaseTable table = provider.getTable(TABLE_NAME);

		table.setColumn(KEY_ADDRESS, "text unique on conflict replace");
		table.setColumn(KEY_TIME, "long");
		table.setColumn(KEY_VALUE, "integer");
		table.setColumn(KEY_MASK, "integer");
	}

	public TestResult(Context context, String address) {
		mAddress = address.toUpperCase();

		try {
			ContentResolver resolver = context.getContentResolver();

			Cursor cursor = resolver.query(CONTENT_URI, PROJECTION, "address=?", new String[] { mAddress }, null);
			if (cursor != null && cursor.moveToFirst()) {
				mTime = cursor.getLong(0);
				mValue = cursor.getInt(1);
				mMask = cursor.getInt(2);
			}
		} catch (Exception e) {
			e.printStackTrace();
			CavanAndroid.showToast(context, R.string.read_test_result_failed);
		}
	}

	public ContentValues getContentValues() {
		ContentValues values = new ContentValues();

		values.put(KEY_ADDRESS, mAddress);
		values.put(KEY_TIME, mTime);
		values.put(KEY_VALUE, mValue);
		values.put(KEY_MASK, mMask);

		return values;
	}

	public Uri save(Context context) {
		mTime = System.currentTimeMillis();
		ContentValues values = getContentValues();

		try {
			ContentResolver resolver = context.getContentResolver();

			return resolver.insert(CONTENT_URI, values);
		} catch (Exception e) {
			e.printStackTrace();
			CavanAndroid.showToast(context, R.string.write_test_result_failed);
		}

		return null;
	}

	public Uri put(Context context, int index, boolean pass) {
		int mask = 1 << index;

		mValue = pass ? (mValue | mask) : (mValue & (~mask));
		mMask |= mask;

		return save(context);
	}

	public int get(int index) {
		int mask = 1 << index;

		if ((mMask & mask) == 0) {
			return -1;
		}

		if ((mValue & mask) == 0) {
			return 0;
		}

		return 1;
	}
}
