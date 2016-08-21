package com.cavan.cavanmain;

import com.cavan.android.CavanDatabaseProvider.CavanDatabaseTable;

public class CavanSettings {

	private static final String TABLE_NAME = "settings";

	public static final String KEY_NAME = "name";
	public static final String KEY_VALUE = "value";

	public static void initDatabaseTable(CavanNotificationProvider provider) {
		CavanDatabaseTable table = provider.getTable(TABLE_NAME);

		table.setColumn(KEY_NAME, "text");
		table.setColumn(KEY_VALUE, "text unique on conflict replace");
	}
}
