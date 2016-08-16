package com.cavan.cavanmain;

import java.util.ArrayList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.cavan.android.CavanAndroid;

import android.app.Notification;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.net.Uri;
import android.provider.BaseColumns;
import android.service.notification.StatusBarNotification;

public class CavanNotification {

	public static final String TABLE_NAME = "notification";
	public static final Uri CONTENT_URI = Uri.withAppendedPath(CavanNotificationProvider.CONTENT_URI, TABLE_NAME);

	public static final String KEY_TIMESTAMP = "timestamp";
	public static final String KEY_PACKAGE = "package";
	public static final String KEY_TITLE = "title";
	public static final String KEY_USER_NAME = "user_name";
	public static final String KEY_GROUP_NAME = "group_name";
	public static final String KEY_CONTENT = "content";

	public static final Pattern sGroupPattern = Pattern.compile("([^\\(]+)\\((.+)\\)\\s*$");

	public static final String[] PROJECTION = {
		KEY_TIMESTAMP, KEY_PACKAGE, KEY_TITLE, KEY_USER_NAME, KEY_GROUP_NAME, KEY_CONTENT
	};

	protected long mTimestamp;
	protected String mPackageName;
	protected String mUserName;
	protected String mGroupName;
	protected String mTitle;
	protected String mContent;

	public CavanNotification(String packageName, String user, String group, String title, String content) {
		mTimestamp = System.currentTimeMillis();
		mPackageName = packageName;
		mUserName = user;
		mGroupName = group;
		mTitle = title;
		mContent = content;
	}

	public CavanNotification(StatusBarNotification sbn) throws Exception {
		mTimestamp = System.currentTimeMillis();
		mPackageName = sbn.getPackageName();

		Notification notification = sbn.getNotification();

		CharSequence title = notification.extras.getCharSequence(Notification.EXTRA_TITLE);
		if (title != null) {
			mTitle = title.toString();
		}

		CharSequence text = notification.tickerText;
		if (text == null) {
			text = notification.extras.getCharSequence(Notification.EXTRA_TEXT);
		}

		if (text == null) {
			throw new Exception("content is empty");
		}

		String content = text.toString();

		CavanAndroid.logE("[" + mTitle + "] ================================================================================");
		CavanAndroid.logE(content);

		String[] contents = content.split(":", 2);

		if (contents.length < 2) {
			mContent = content.trim();
		} else {
			String name = contents[0].trim();
			Matcher matcher = sGroupPattern.matcher(name);
			if (matcher.find()) {
				mUserName = matcher.group(1);
				mGroupName = matcher.group(2);
			} else {
				mUserName = name;
			}

			mContent = contents[1].trim();
		}
	}

	public CavanNotification(Cursor cursor) {
		int column = 0;

		mTimestamp = cursor.getLong(column++);
		mPackageName = cursor.getString(column++);
		mTitle = cursor.getString(column++);
		mUserName = cursor.getString(column++);
		mGroupName = cursor.getString(column++);
		mContent = cursor.getString(column++);
	}

	// ================================================================================

	public long getTimestamp() {
		return mTimestamp;
	}

	public void setTimestamp(long timestamp) {
		mTimestamp = timestamp;
	}

	public String getPackageName() {
		return mPackageName;
	}

	public void setPackageName(String packageName) {
		mPackageName = packageName;
	}

	public CharSequence getUserName() {
		return mUserName;
	}

	public void setUserName(String name) {
		mUserName = name;
	}

	public CharSequence getGroupName() {
		return mGroupName;
	}

	public void setGroupName(String name) {
		mGroupName = name;
	}

	public CharSequence getTitle() {
		return mTitle;
	}

	public void setTitle(String title) {
		mTitle = title;
	}

	public CharSequence getContent() {
		return mContent;
	}

	public void setContent(String content) {
		mContent = content;
	}

	// ================================================================================

	public ContentValues getContentValues() {
		ContentValues values = new ContentValues(6);

		values.put(KEY_TIMESTAMP, mTimestamp);

		if (mPackageName != null) {
			values.put(KEY_PACKAGE, mPackageName);
		}

		if (mTitle != null) {
			values.put(KEY_TITLE, mTitle.toString());
		}

		if (mUserName != null) {
			values.put(KEY_USER_NAME, mUserName.toString());
		}

		if (mGroupName != null) {
			values.put(KEY_GROUP_NAME, mGroupName.toString());
		}

		if (mContent != null) {
			values.put(KEY_CONTENT, mContent.toString());
		}

		return values;
	}

	public Uri insert(ContentResolver resolver) {
		return resolver.insert(CONTENT_URI, getContentValues());
	}

	public static void createTable(SQLiteDatabase db) {
		db.execSQL("drop table if exists " + TABLE_NAME);

		StringBuilder builder = new StringBuilder(64);

		builder.append("create table ");
		builder.append(TABLE_NAME);
		builder.append("(");
		builder.append(BaseColumns._ID);
		builder.append(" integer primary key autoincrement, ");
		builder.append(KEY_TIMESTAMP);
		builder.append(" date not null, ");
		builder.append(KEY_PACKAGE);
		builder.append(" text, ");
		builder.append(KEY_TITLE);
		builder.append(" text, ");
		builder.append(KEY_USER_NAME);
		builder.append(" text, ");
		builder.append(KEY_GROUP_NAME);
		builder.append(" text, ");
		builder.append(KEY_CONTENT);
		builder.append(" text)");

		db.execSQL(builder.toString());
	}

	public static ArrayList<CavanNotification> parseCursor(Cursor cursor) {
		ArrayList<CavanNotification> notifications = new ArrayList<CavanNotification>();

		if (cursor != null && cursor.moveToFirst()) {
			do {
				notifications.add(new CavanNotification(cursor));
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
		return delete(resolver, KEY_PACKAGE + "=?", new String[] { pkgName } );
	}

	public static int deleteByUser(ContentResolver resolver, String user) {
		return delete(resolver, KEY_USER_NAME + "=?", new String[] { user } );
	}

	public static int deleteByGroup(ContentResolver resolver, String group) {
		return delete(resolver, KEY_GROUP_NAME + "=?", new String[] { group } );
	}

	public static ArrayList<CavanNotification> query(ContentResolver resolver, String[] projection, String selection, String[] selectionArgs) {
		Cursor cursor = resolver.query(CONTENT_URI, projection, selection, selectionArgs, null);
		return parseCursor(cursor);
	}

	public static ArrayList<CavanNotification> queryAll(ContentResolver resolver) {
		return query(resolver, PROJECTION, null, null);
	}

	public static ArrayList<CavanNotification> queryByPackage(ContentResolver resolver, String pkgName) {
		return query(resolver, PROJECTION, KEY_PACKAGE + "=?", new String[] { pkgName });
	}

	public static ArrayList<CavanNotification> queryByUser(ContentResolver resolver, String user) {
		return query(resolver, PROJECTION, KEY_USER_NAME + "=?", new String[] { user });
	}

	public static ArrayList<CavanNotification> queryByGroup(ContentResolver resolver, String group) {
		return query(resolver, PROJECTION, KEY_GROUP_NAME + "=?", new String[] { group });
	}

	@Override
	public String toString() {
		StringBuilder builder = new StringBuilder();

		builder.append("timestamp = ");
		builder.append(mTimestamp);

		if (mPackageName != null) {
			builder.append(", package = ");
			builder.append(mPackageName);
		}

		if (mTitle != null) {
			builder.append(", title = ");
			builder.append(mTitle);
		}

		if (mUserName != null) {
			builder.append(", user = ");
			builder.append(mUserName);
		}

		if (mGroupName != null) {
			builder.append(", group = ");
			builder.append(mGroupName);
		}

		if (mContent != null) {
			builder.append(", content = ");
			builder.append(mContent);
		}

		return builder.toString();
	}
}
