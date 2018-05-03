package com.cavan.cavanmain;

import android.database.sqlite.SQLiteDatabase;
import android.net.Uri;

import com.cavan.accessibility.CavanNotification;
import com.cavan.android.CavanDatabaseProvider;

public class CavanNotificationProvider extends CavanDatabaseProvider {

	public static final String AUTHORIT = "com.cavan.notification.provider";
	public static final Uri CONTENT_URI = Uri.parse("content://" + AUTHORIT);

	public static final String DB_NAME = "CavanMain.db";
	public static final int DB_VERSION = 5;

	@Override
	protected void initTables() {
		CavanNotification.initDatabaseTable(this);
		CavanSetting.initDatabaseTable(this);
		CavanFilter.initDatabaseTable(this);
		CavanUserInfo.initDatabaseTable(this);
		CavanSubjectInfo.initDatabaseTable(this);
	}

	@Override
	protected String getAuthority() {
		return AUTHORIT;
	}

	@Override
	protected String getDatabaseName() {
		return DB_NAME;
	}

	@Override
	protected int getDatabaseVersion() {
		return DB_VERSION;
	}

	@Override
	protected void onDatabaseUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
		if (oldVersion < 4) {
			CavanDatabaseTable table = getTable(CavanUserInfo.TABLE_NAME);
			if (table != null) {
				db.execSQL(table.buildCreateTableSql());
			}
		}

		if (oldVersion < 5) {
			CavanDatabaseTable table = getTable(CavanSubjectInfo.TABLE_NAME);
			if (table != null) {
				db.execSQL(table.buildCreateTableSql());
			}
		}
	}
}
