package com.cavan.jwaootoyfactorytest;

import android.net.Uri;
import android.os.Environment;

import com.cavan.android.CavanDatabaseProvider;

public class DatabaseProvider extends CavanDatabaseProvider {

	public static final String AUTHORIT = "com.cavan.jwaootoyfactorytest";
	public static final Uri CONTENT_URI = Uri.parse("content://" + AUTHORIT);

	private static final String DB_NAME = Environment.getExternalStorageDirectory().getPath() + "/JwaooFactoryModel06.db";
	private static final int DB_VERSION = 1;

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
	protected void initTables() {
		TestResult.initDatabaseTable(this);
	}
}
