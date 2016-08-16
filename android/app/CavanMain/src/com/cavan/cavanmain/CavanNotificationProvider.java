package com.cavan.cavanmain;

import android.content.ContentProvider;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;
import android.provider.BaseColumns;

public class CavanNotificationProvider extends ContentProvider {

	public static final String AUTHORIT = "com.cavan.notification.provider";
	public static final Uri CONTENT_URI = Uri.parse("content://" + AUTHORIT);

	private static final String DB_NAME = "notification.db";
	private static final int DB_VERSION = 1;

	private static final int URI_NOTIFICATION = 0x1000;
	private static final int URI_NOTIFICATION_ID = URI_NOTIFICATION + 1;

	private static final UriMatcher sUriMatcher = new UriMatcher(UriMatcher.NO_MATCH);

	static {
		sUriMatcher.addURI(AUTHORIT, CavanNotification.TABLE_NAME, URI_NOTIFICATION);
		sUriMatcher.addURI(AUTHORIT, CavanNotification.TABLE_NAME + "/#", URI_NOTIFICATION_ID);
	}

	private DatabaseHelper mDatabaseHelper;

	@Override
	public boolean onCreate() {
		mDatabaseHelper = new DatabaseHelper();
		return true;
	}

	@Override
	public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs, String sortOrder) {
		SQLiteQueryBuilder builder = new SQLiteQueryBuilder();

		switch (sUriMatcher.match(uri)) {
		case URI_NOTIFICATION_ID:
			builder.appendWhere(BaseColumns._ID + "=");
			builder.appendWhere(uri.getPathSegments().get(1));
		case URI_NOTIFICATION:
			builder.setTables(CavanNotification.TABLE_NAME);
			break;

		default:
			return null;
		}

		SQLiteDatabase db = mDatabaseHelper.getReadableDatabase();

		return builder.query(db, projection, selection, selectionArgs, null, null, sortOrder);
	}

	@Override
	public String getType(Uri uri) {
		return null;
	}

	@Override
	public Uri insert(Uri uri, ContentValues values) {
		if (sUriMatcher.match(uri) != URI_NOTIFICATION) {
			return null;
		}

		SQLiteDatabase database = mDatabaseHelper.getWritableDatabase();
		long id = database.insert(CavanNotification.TABLE_NAME, null, values);
		if (id < 0) {
			return null;
		}

		uri = ContentUris.withAppendedId(uri, id);
		getContext().getContentResolver().notifyChange(uri, null);

		return uri;

	}

	@Override
	public int delete(Uri uri, String selection, String[] selectionArgs) {
		SQLiteDatabase db = mDatabaseHelper.getWritableDatabase();

		switch (sUriMatcher.match(uri)) {
		case URI_NOTIFICATION_ID:
			selection = BaseColumns._ID + "=" + uri.getPathSegments().get(1);
			selectionArgs = null;
			break;

		case URI_NOTIFICATION:
			break;

		default:
			return 0;
		}

		return db.delete(CavanNotification.TABLE_NAME, selection, selectionArgs);
	}

	@Override
	public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs) {
		if (sUriMatcher.match(uri) != URI_NOTIFICATION_ID) {
			return 0;
		}

		SQLiteDatabase db = mDatabaseHelper.getWritableDatabase();
		int count = db.update(CavanNotification.TABLE_NAME, values, BaseColumns._ID + "=" + uri.getPathSegments().get(1), null);
		if (count > 0) {
			getContext().getContentResolver().notifyChange(uri, null);
		}

		return count;

	}

	public class DatabaseHelper extends SQLiteOpenHelper {

		public DatabaseHelper() {
			super(getContext(), DB_NAME, null, DB_VERSION);
		}

		@Override
		public void onCreate(SQLiteDatabase db) {
			CavanNotification.createTable(db);
		}

		@Override
		public void onDowngrade(SQLiteDatabase db, int oldVersion, int newVersion) {
			onCreate(db);
		}

		@Override
		public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
			onCreate(db);
		}
	}
}
