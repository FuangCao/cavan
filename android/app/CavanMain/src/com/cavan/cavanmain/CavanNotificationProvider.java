package com.cavan.cavanmain;

import android.net.Uri;

import com.cavan.android.CavanDatabaseProvider;

public class CavanNotificationProvider extends CavanDatabaseProvider {

	public static final String AUTHORIT = "com.cavan.notification.provider";
	public static final Uri CONTENT_URI = Uri.parse("content://" + AUTHORIT);

	private static final String DB_NAME = "notification.db";
	private static final int DB_VERSION = 1;

	private CavanDatabaseTable mTableNotitication = new CavanDatabaseTable("notification", 0x1000);

	@Override
	protected boolean doInitialize() {
		CavanNotification.initTableColumns(mTableNotitication);
		mTableNotitication.attachToProvider();
		return false;
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
}
