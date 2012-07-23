package com.eavoo.cavan;

import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

public class SQLiteOpenHelperMessage extends SQLiteOpenHelper
{
	private static final String TAG = "SQLiteOpenHelper";
	private static final String DEFAULT_DB_NAME = "eavoo.db";
	private static final String DEFAULT_TABLE_NAME = "message";

	public SQLiteOpenHelperMessage(Context context, String name)
	{
		super(context, name == null ? DEFAULT_DB_NAME : name, null, 2);
	}

	@Override
	public void onCreate(SQLiteDatabase db)
	{
		StringBuilder sqlBuilder = new StringBuilder("create table " + DEFAULT_TABLE_NAME + "(");
		sqlBuilder.append("id integer primary key autoincrement, ");
		sqlBuilder.append("date text not null, ");
		sqlBuilder.append("phone text not null, ");
		sqlBuilder.append("content text not null");
		sqlBuilder.append(")");
		String sqlString = sqlBuilder.toString();
		Log.i(TAG, "SQL = " + sqlString);

		db.execSQL(sqlString);
	}

	@Override
	public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion)
	{
		String sqlString = "drop table " + DEFAULT_TABLE_NAME;

		Log.i(TAG, "SQL = " + sqlString);
		db.execSQL(sqlString);
		onCreate(db);
	}

	public long insert(EavooShortMessage message)
	{
		Log.i(TAG, "insert: " + message);

		SQLiteDatabase database = getWritableDatabase();
		return database.insert(DEFAULT_TABLE_NAME, null, message.toContentValues());
	}

	public int delete(int id)
	{
		SQLiteDatabase database = getWritableDatabase();
		String[] args = new String[] {Integer.toString(id)};
		return database.delete(DEFAULT_TABLE_NAME, "id=?", args);
	}

	public Cursor quertAll()
	{
		SQLiteDatabase database = getReadableDatabase();
		String columns[] = {"id", "date", "phone", "content"};
		return database.query(DEFAULT_TABLE_NAME, columns, null, null, null, null, null);
	}
}
