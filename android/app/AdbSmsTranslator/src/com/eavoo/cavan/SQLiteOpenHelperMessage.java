package com.eavoo.cavan;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

public class SQLiteOpenHelperMessage extends SQLiteOpenHelper
{
	private static final String TAG = "SQLiteOpenHelper";
	private String mTableName;

	public SQLiteOpenHelperMessage(Context context, String dbName, String tableName)
	{
		super(context, dbName, null, 2);
		mTableName = tableName;
	}

	public String getTableName()
	{
		return mTableName;
	}

	@Override
	public void onCreate(SQLiteDatabase db)
	{
		EavooShortMessage.CreateDatabaseTable(db, mTableName);
	}

	@Override
	public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion)
	{
		String sqlString = "drop table " + mTableName;

		Log.i(TAG, "SQL = " + sqlString);
		db.execSQL(sqlString);
		onCreate(db);
	}
}
