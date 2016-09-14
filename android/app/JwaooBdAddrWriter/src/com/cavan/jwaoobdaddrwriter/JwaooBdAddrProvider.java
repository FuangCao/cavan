package com.cavan.jwaoobdaddrwriter;

import android.net.Uri;

import com.cavan.android.CavanDatabaseProvider;

public class JwaooBdAddrProvider extends CavanDatabaseProvider {

	public static final String AUTHORIT = "com.jwaoo.bdaddr.provider";
	public static final Uri CONTENT_URI = Uri.parse("content://" + AUTHORIT);

	private static final String DB_NAME = "bdaddr.db";
	private static final int DB_VERSION = 1;

	@Override
	protected void initTables() {
		JwaooBdAddr.initDatabaseTable(this);
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
