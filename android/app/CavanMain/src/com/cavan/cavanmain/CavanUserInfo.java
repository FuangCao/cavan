package com.cavan.cavanmain;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.database.Cursor;
import android.net.Uri;
import android.util.Base64;

import com.cavan.android.CavanDatabaseProvider;
import com.cavan.android.CavanDatabaseProvider.CavanDatabaseTable;
import com.cavan.java.CavanJava;

public class CavanUserInfo {

	public static final String TABLE_NAME = "users";
	public static final Uri CONTENT_URI = Uri.withAppendedPath(CavanNotificationProvider.CONTENT_URI, TABLE_NAME);

	public static final String KEY_PACKAGE = "package";
	public static final String KEY_ACCOUNT = "account";
	public static final String KEY_PASSWORD = "password";

	private static final String[] PROJECTION = {
		CavanDatabaseProvider.KEY_ID, KEY_PACKAGE, KEY_ACCOUNT, KEY_PASSWORD
	};

	private long mDatabaseId;
	private String mPackage;
	private String mAccount;
	private String mPassword;
	private boolean mChecked;

	public CavanUserInfo(String pkg, String account, String password) {
		mPackage = pkg;
		mAccount = account;
		mPassword = password;
	}

	public CavanUserInfo(Cursor cursor) {
		parse(cursor);
	}

	public void parse(Cursor cursor) {
		mDatabaseId = cursor.getLong(0);
		mPackage = cursor.getString(1);
		mAccount = cursor.getString(2);
		setPasswordAes(cursor.getString(3));
	}

	public long getDatabaseId() {
		return mDatabaseId;
	}

	public void setDatabaseId(long id) {
		mDatabaseId = id;
	}

	public String getPackage() {
		return mPackage;
	}

	public void setPackage(String package1) {
		mPackage = package1;
	}

	public String getAccount() {
		return mAccount;
	}

	public void setAccount(String account) {
		mAccount = account;
	}

	public String getPassword() {
		return mPassword;
	}

	public void setPassword(String password) {
		mPassword = password;
	}

	public boolean isChecked() {
		return mChecked;
	}

	public void setChecked(boolean checked) {
		mChecked = checked;
	}

	public String getPasswordAes() {
		byte[] bytes = CavanJava.AesEncrypt(mPassword);
		return Base64.encodeToString(bytes, 0);
	}

	public void setPasswordAes(String password) {
		byte[] bytes = CavanJava.AesDecrypt(Base64.decode(password, 0));
		if (bytes != null) {
			mPassword = new String(bytes);
		}
	}

	public ContentValues getContentValues() {
		ContentValues values = new ContentValues();

		values.put(KEY_PACKAGE, mPackage);
		values.put(KEY_ACCOUNT, mAccount);
		values.put(KEY_PASSWORD, getPasswordAes());

		return values;
	}

	public Uri save(ContentResolver resolver) {
		return resolver.insert(CONTENT_URI, getContentValues());
	}

	public int delete(ContentResolver resolver) {
		String where = CavanDatabaseProvider.KEY_ID + '=' + mDatabaseId;
		return resolver.delete(CONTENT_URI, where, null);
	}

	@Override
	public String toString() {
		StringBuilder builder = new StringBuilder();

		builder.append('[');
		builder.append(KEY_PACKAGE).append(':').append(mPackage).append(", ");
		builder.append(KEY_ACCOUNT).append(':').append(mAccount).append(", ");
		builder.append(KEY_PASSWORD).append(':').append(mPassword);
		builder.append(']');

		return builder.toString();
	}

	public static void initDatabaseTable(CavanNotificationProvider provider) {
		CavanDatabaseTable table = provider.getTable(TABLE_NAME);

		table.setColumn(KEY_PACKAGE, "text");
		table.setColumn(KEY_ACCOUNT, "text");
		table.setColumn(KEY_PASSWORD, "text");
		table.addSql("unique (" + KEY_PACKAGE + "," + KEY_ACCOUNT + ") on conflict replace");
	}

	public static CavanUserInfo[] read(Cursor cursor) {
		CavanUserInfo[] users = new CavanUserInfo[cursor.getCount()];

		for (int i = 0; i < users.length; i++) {
			if (cursor.moveToPosition(i)) {
				users[i] = new CavanUserInfo(cursor);
			}
		}

		return users;
	}

	public static CavanUserInfo[] query(ContentResolver resolver, String pkg, String account) {
		String selection;
		String[] selectionArgs;

		if (pkg != null) {
			if (account != null) {
				selection = KEY_PACKAGE + "=? and " + KEY_ACCOUNT + "=?";
				selectionArgs = new String[] { pkg, account };
			} else {
				selection = KEY_PACKAGE + "=?";
				selectionArgs = new String[] { pkg };
			}
		} else if (account != null) {
			selection = KEY_ACCOUNT + "=?";
			selectionArgs = new String[] { account };
		} else {
			selection = null;
			selectionArgs = null;
		}

		Cursor cursor = resolver.query(CONTENT_URI, PROJECTION, selection, selectionArgs, null);
		if (cursor != null) {
			return read(cursor);
		}

		return null;
	}
}
