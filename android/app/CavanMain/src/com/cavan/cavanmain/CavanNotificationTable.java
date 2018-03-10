package com.cavan.cavanmain;

import java.util.ArrayList;

import android.content.ContentResolver;
import android.database.Cursor;
import android.net.Uri;

import com.cavan.accessibility.CavanNotification;

public class CavanNotificationTable {

	public static final Uri CONTENT_URI = Uri.withAppendedPath(CavanNotificationProvider.CONTENT_URI, CavanNotification.TABLE_NAME);

	protected CavanNotification mNotification;

	public CavanNotificationTable(CavanNotification notification) {
		mNotification = notification;
	}

	public CavanNotificationTable(Cursor cursor) throws Exception {
		this(new CavanNotification(cursor));
	}

	public CavanNotification getNotification() {
		return mNotification;
	}

	public void setNotification(CavanNotification notification) {
		mNotification = notification;
	}

	public Uri insert(ContentResolver resolver) {
		return resolver.insert(CONTENT_URI, mNotification.getContentValues());
	}

	public static ArrayList<CavanNotificationTable> parseCursor(Cursor cursor) {
		ArrayList<CavanNotificationTable> notifications = new ArrayList<CavanNotificationTable>();

		if (cursor != null && cursor.moveToFirst()) {
			do {
				try {
					notifications.add(new CavanNotificationTable(cursor));
				} catch (Exception e) {
					break;
				}
			} while (cursor.moveToNext());
		}

		return notifications;
	}

	public static int delete(ContentResolver resolver, String where, String[] selectionArgs) {
		return resolver.delete(CONTENT_URI, where, selectionArgs);
	}

	public static int deleteAll(ContentResolver resolver) {
		return delete(resolver, null, null);
	}

	public static int deleteByPackage(ContentResolver resolver, String pkgName) {
		return delete(resolver, CavanNotification.KEY_PACKAGE + "=?", new String[] { pkgName } );
	}

	public static int deleteByUser(ContentResolver resolver, String user) {
		return delete(resolver, CavanNotification.KEY_USER_NAME + "=?", new String[] { user } );
	}

	public static int deleteByGroup(ContentResolver resolver, String group) {
		return delete(resolver, CavanNotification.KEY_GROUP_NAME + "=?", new String[] { group } );
	}

	public static Cursor query(ContentResolver resolver, String[] projection, String selection, String[] selectionArgs, String sortOrder) {
		return resolver.query(CONTENT_URI, projection, selection, selectionArgs, sortOrder);
	}

	public static Cursor query(ContentResolver resolver, Uri uri, String selection, String[] selectionArgs, String sortOrder) {
		return resolver.query(uri, CavanNotification.PROJECTION, selection, selectionArgs, sortOrder);
	}

	public static Cursor query(ContentResolver resolver, String selection, String[] selectionArgs, String sortOrder) {
		return resolver.query(CONTENT_URI, CavanNotification.PROJECTION, selection, selectionArgs, sortOrder);
	}

	public static Cursor queryAll(ContentResolver resolver, String[] projection, String sortOrder) {
		return query(resolver, projection, null, null, sortOrder);
	}

	public static Cursor queryAll(ContentResolver resolver, String sortOrder) {
		return query(resolver, null, null, sortOrder);
	}

	public static ArrayList<CavanNotificationTable> queryNotification(ContentResolver resolver, String selection, String[] selectionArgs, String sortOrder) {
		Cursor cursor = query(resolver, CavanNotification.PROJECTION, selection, selectionArgs, sortOrder);
		return parseCursor(cursor);
	}

	public static ArrayList<CavanNotificationTable> queryNotificationAll(ContentResolver resolver, String sortOrder) {
		return queryNotification(resolver, null, null, sortOrder);
	}

	public static ArrayList<CavanNotificationTable> queryNotificationByPackage(ContentResolver resolver, String pkgName, String sortOrder) {
		return queryNotification(resolver, CavanNotification.KEY_PACKAGE + "=?", new String[] { pkgName }, sortOrder);
	}

	public static ArrayList<CavanNotificationTable> queryByUser(ContentResolver resolver, String user, String sortOrder) {
		return queryNotification(resolver, CavanNotification.KEY_USER_NAME + "=?", new String[] { user }, sortOrder);
	}

	public static ArrayList<CavanNotificationTable> queryNotificationByGroup(ContentResolver resolver, String group, String sortOrder) {
		return queryNotification(resolver, CavanNotification.KEY_GROUP_NAME + "=?", new String[] { group }, sortOrder);
	}
}
