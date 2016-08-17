package com.cavan.android;

import java.util.HashMap;

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

public abstract class CavanDatabaseProvider extends ContentProvider {

	protected abstract String getDatabaseName();
	protected abstract int getDatabaseVersion();
	protected abstract String getAuthority();
	protected abstract void initTables();

	private CavanDatabaseHelper mDatabaseHelper;
	private UriMatcher mUriMatcher = new UriMatcher(UriMatcher.NO_MATCH);
	private HashMap<String, CavanDatabaseTable> mTableNameMap = new HashMap<String, CavanDatabaseTable>();
	private HashMap<Integer, CavanDatabaseTable> mTableCodeMap = new HashMap<Integer, CavanDatabaseTable>();

	public class CavanDatabaseColumn {

		private String mName;
		private String mType;

		private CavanDatabaseColumn(String name, String type) {
			super();

			mName = name;
			mType = type;
		}

		public String getName() {
			return mName;
		}

		public void setName(String name) {
			mName = name;
		}

		public String getType() {
			return mType;
		}

		public void setType(String type) {
			mType = type;
		}

		public String buildSql() {
			return mName + " " + mType;
		}

		public void buildSql(StringBuilder builder) {
			builder.append(mName);
			builder.append(" ");
			builder.append(mType);
		}
	}

	public class CavanDatabaseTable {

		private String mName;
		private int mBaseCode;
		private HashMap<String, CavanDatabaseColumn> mColumnNameMap = new HashMap<String, CavanDatabaseColumn>();

		private CavanDatabaseTable(String name, int code) {
			mName = name;
			mBaseCode = code;

			addURI(null, 0);
			addURI("#", 1);
			setColumn(BaseColumns._ID, "integer primary key autoincrement");
		}

		public String getName() {
			return mName;
		}

		public int getBaseCode() {
			return mBaseCode;
		}

		public int getRelCode(int code) {
			return mBaseCode - code;
		}

		public int getAbsCode(int code) {
			return mBaseCode + code;
		}

		public String getAbsPath(String path) {
			return mName + "/" + path;
		}

		public CavanDatabaseColumn getColumn(String name) {
			return mColumnNameMap.get(name);
		}

		public CavanDatabaseColumn setColumn(String name, String type) {
			CavanDatabaseColumn column = getColumn(name);
			if (column == null) {
				column = new CavanDatabaseColumn(name, type);
				mColumnNameMap.put(name, column);
			} else {
				column.setType(type);
			}

			return column;
		}

		public String buildDropTableSql() {
			return "drop table if exists  " + mName;
		}

		public String buildCreateTableSql() {
			StringBuilder builder = new StringBuilder("create table ");
			builder.append(mName);
			builder.append("(");

			int index = 0;

			for (CavanDatabaseColumn column : mColumnNameMap.values()) {
				if (index > 0) {
					builder.append(",");
				}

				column.buildSql(builder);
				index++;
			}

			builder.append(")");

			return builder.toString();
		}

		public void addURI(String path, int code) {
			if (path == null) {
				path = mName;
				code = mBaseCode;
			} else {
				path = getAbsPath(path);
				code = getAbsCode(code);
			}

			mUriMatcher.addURI(getAuthority(), path, code);
			mTableCodeMap.put(code, this);
		}

		public String getType(Uri uri, int code) {
			return null;
		}

		public Cursor query(Uri uri, int code, String[] projection, String selection, String[] selectionArgs, String sortOrder) {
			SQLiteQueryBuilder builder = new SQLiteQueryBuilder();

			builder.setTables(mName);

			if (code == 1) {
				builder.appendWhere(BaseColumns._ID + "=");
				builder.appendWhere(uri.getPathSegments().get(1));
			}

			SQLiteDatabase db = mDatabaseHelper.getReadableDatabase();

			return builder.query(db, projection, selection, selectionArgs, null, null, sortOrder);
		}

		public Uri insert(Uri uri, int code, ContentValues values) {
			if (code != 0) {
				return null;
			}

			SQLiteDatabase database = mDatabaseHelper.getWritableDatabase();
			long id = database.insert(mName, null, values);
			if (id < 0) {
				return null;
			}

			uri = ContentUris.withAppendedId(uri, id);
			getContext().getContentResolver().notifyChange(uri, null);

			return uri;
		}

		public int delete(Uri uri, int code, String selection, String[] selectionArgs) {
			SQLiteDatabase db = mDatabaseHelper.getWritableDatabase();

			if (code == 1) {
				selection = BaseColumns._ID + "=" + uri.getPathSegments().get(1);
				selectionArgs = null;
			}

			return db.delete(mName, selection, selectionArgs);
		}

		public int update(Uri uri, int code, ContentValues values, String selection, String[] selectionArgs) {
			SQLiteDatabase db = mDatabaseHelper.getWritableDatabase();

			if (code == 1) {
				selection = BaseColumns._ID + "=" + uri.getPathSegments().get(1);
				selectionArgs = null;
			}

			return db.delete(mName, selection, selectionArgs);
		}
	}

	public class CavanDatabaseHelper extends SQLiteOpenHelper {

		public CavanDatabaseHelper() {
			super(getContext(), CavanDatabaseProvider.this.getDatabaseName(), null, CavanDatabaseProvider.this.getDatabaseVersion());
		}

		@Override
		public void onCreate(SQLiteDatabase db) {
			for (CavanDatabaseTable table : mTableNameMap.values()) {
				db.execSQL(table.buildCreateTableSql());
			}
		}

		@Override
		public void onDowngrade(SQLiteDatabase db, int oldVersion, int newVersion) {
			for (CavanDatabaseTable table : mTableNameMap.values()) {
				db.execSQL(table.buildDropTableSql());
				db.execSQL(table.buildCreateTableSql());
			}
		}

		@Override
		public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
			onDowngrade(db, oldVersion, newVersion);
		}
	}

	public CavanDatabaseProvider() {
		super();
		initTables();
	}

	public CavanDatabaseTable getTable(String name) {
		CavanDatabaseTable table = mTableNameMap.get(name);
		if (table != null) {
			return table;
		}

		table = new CavanDatabaseTable(name, mTableNameMap.size() << 10);
		mTableNameMap.put(name, table);

		return table;
	}

	public SQLiteDatabase getReadableDatabase() {
		return mDatabaseHelper.getReadableDatabase();
	}

	public SQLiteDatabase getWritableDatabase() {
		return mDatabaseHelper.getWritableDatabase();
	}

	@Override
	public boolean onCreate() {
		mDatabaseHelper = new CavanDatabaseHelper();
		return true;
	}

	@Override
	public String getType(Uri uri) {
		int code = mUriMatcher.match(uri);
		CavanDatabaseTable table = mTableCodeMap.get(code);
		if (table == null) {
			return null;
		}

		return table.getType(uri, table.getRelCode(code));
	}

	@Override
	public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs, String sortOrder) {
		int code = mUriMatcher.match(uri);
		CavanDatabaseTable table = mTableCodeMap.get(code);
		if (table == null) {
			return null;
		}

		return table.query(uri, table.getRelCode(code), projection, selection, selectionArgs, sortOrder);
	}

	@Override
	public Uri insert(Uri uri, ContentValues values) {
		int code = mUriMatcher.match(uri);
		CavanDatabaseTable table = mTableCodeMap.get(code);
		if (table == null) {
			return null;
		}

		return table.insert(uri, table.getRelCode(code), values);
	}

	@Override
	public int delete(Uri uri, String selection, String[] selectionArgs) {
		int code = mUriMatcher.match(uri);
		CavanDatabaseTable table = mTableCodeMap.get(code);
		if (table == null) {
			return 0;
		}

		return table.delete(uri, table.getRelCode(code), selection, selectionArgs);
	}

	@Override
	public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs) {
		int code = mUriMatcher.match(uri);
		CavanDatabaseTable table = mTableCodeMap.get(code);
		if (table == null) {
			return 0;
		}

		return table.update(uri, table.getRelCode(code), values, selection, selectionArgs);
	}
}
