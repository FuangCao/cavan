package com.cavan.jwaoobdaddrwriter;

import java.util.List;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.database.Cursor;
import android.net.Uri;

import com.cavan.android.CavanDatabaseProvider;
import com.cavan.android.CavanDatabaseProvider.CavanDatabaseTable;
import com.cavan.java.CavanByteCache;
import com.cavan.java.CavanString;

public class JwaooBdAddr {

	private static final String TABLE_NAME = "addresses";
	public static final Uri CONTENT_URI = Uri.withAppendedPath(JwaooBdAddrProvider.CONTENT_URI, TABLE_NAME);

	public static final String KEY_TIMESTAMP = "timestamp";
	public static final String KEY_ADDRESS = "address";
	public static final String KEY_COUNT = "count";

	public static final String[] PROJECTION = {
		CavanDatabaseProvider.KEY_ID, KEY_TIMESTAMP, KEY_ADDRESS, KEY_COUNT
	};

	public static void initDatabaseTable(JwaooBdAddrProvider provider) {
		CavanDatabaseTable table = provider.getTable(TABLE_NAME);

		table.setColumn(KEY_TIMESTAMP, "long");
		table.setColumn(KEY_ADDRESS, "long");
		table.setColumn(KEY_COUNT, "integer");
	}

	public static Cursor queryCursor(ContentResolver resolver) {
		return resolver.query(CONTENT_URI, PROJECTION, null, null, KEY_COUNT + " desc limit 1");
	}

	public static JwaooBdAddr query(ContentResolver resolver) {
		Cursor cursor = queryCursor(resolver);
		if (cursor != null && cursor.moveToFirst()) {
			return new JwaooBdAddr(cursor);
		}

		return null;
	}

	public static long queryAddrCount(ContentResolver resolver) {
		Cursor cursor = resolver.query(CONTENT_URI, new String[] { "sum(" + KEY_COUNT + ")" }, null, null, null);
		if (cursor != null && cursor.moveToFirst()) {
			return cursor.getLong(0);
		}

		return 0;
	}

	private long mIdentify;
	private long mTimestamp;
	private long mAddress;
	private int mCount;

	public JwaooBdAddr(long address, int count) {
		mIdentify = -1;
		mTimestamp = System.currentTimeMillis();
		mAddress = address;
		mCount = count;
	}

	public JwaooBdAddr(Cursor cursor) {
		mIdentify = cursor.getLong(0);
		mTimestamp = cursor.getLong(1);
		mAddress = cursor.getLong(2);
		mCount = cursor.getInt(3);
	}

	public long getTimestamp() {
		return mTimestamp;
	}

	public void setTimestamp(long timestamp) {
		mTimestamp = timestamp;
	}

	public long getAddress() {
		return mAddress;
	}

	public void setAddress(long address) {
		mAddress = address;
	}

	public int getCount() {
		return mCount;
	}

	public void setCount(int count) {
		mCount = count;
	}

	public Uri getIdentifyUri() {
		return Uri.withAppendedPath(CONTENT_URI, Long.toString(mIdentify));
	}

	public ContentValues getContentValues() {
		ContentValues values = new ContentValues();

		values.put(KEY_TIMESTAMP, mTimestamp);
		values.put(KEY_ADDRESS, mAddress);
		values.put(KEY_COUNT, mCount);

		return values;
	}

	public Uri fflush(ContentResolver resolver) {
		if (mCount > 0) {
			if (mIdentify < 0) {
				Uri uri = resolver.insert(CONTENT_URI, getContentValues());
				if (uri == null) {
					return null;
				}

				List<String> segments = uri.getPathSegments();
				mIdentify = Long.parseLong(segments.get(segments.size() - 1));

				return uri;
			} else {
				Uri uri = getIdentifyUri();
				if (resolver.update(uri, getContentValues(), null, null) > 0) {
					return uri;
				}
			}
		} else if (mIdentify > 0) {
			Uri uri = getIdentifyUri();
			if (resolver.delete(uri, null, null) > 0) {
				return uri;
			}
		}

		return null;
	}

	public JwaooBdAddr alloc(ContentResolver resolver) {
		if (mCount > 0) {
			long address = mAddress;

			mAddress++;
			mCount--;

			if (fflush(resolver) != null) {
				return new JwaooBdAddr(address, 1);
			}

			mAddress--;
			mCount++;
		}

		return null;
	}

	public boolean recycle(ContentResolver resolver) {
		return fflush(resolver) != null;
	}

	public byte[] getBytes() {
		CavanByteCache cache = new CavanByteCache(6);
		cache.writeValue8((byte) 0x80);
		cache.writeValue8((byte) 0xEA);
		cache.writeValueBe32((int) mAddress);
		return cache.getBytes();
	}

	@Override
	public String toString() {
		StringBuilder builder = new StringBuilder();

		builder.append(CavanString.fromBdAddr(getBytes()));
		builder.append(", count = ");
		builder.append(mCount);

		return builder.toString();
	}
}
