package com.cavan.wifi;

import java.io.File;
import java.net.URI;
import java.util.ArrayList;

import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanAndroidFile;

@SuppressWarnings("serial")
public class WifiConfigFile extends CavanAndroidFile {

	public WifiConfigFile(File dir, String name) {
		super(dir, name);
	}

	public WifiConfigFile(String dirPath, String name) {
		super(dirPath, name);
	}

	public WifiConfigFile(String path) {
		super(path);
	}

	public WifiConfigFile(URI uri) {
		super(uri);
	}

	public ArrayList<String> getAccessPoints() {
		ArrayList<String> points = new ArrayList<String>();
		SQLiteDatabase database = null;

		try {
			database = SQLiteDatabase.openDatabase(getAbsolutePath(), null, SQLiteDatabase.OPEN_READONLY);
			if (database == null) {
				CavanAndroid.eLog("openDatabase: " + getAbsolutePath());
				return points;
			}

			Cursor cursor = database.query("wifiConfig", new String[] { "ssid", "psk" }, null, null, null, null, null);
			if (cursor != null && cursor.moveToFirst()) {
				do {
					points.add(cursor.getString(0) + " - " + cursor.getString(1));
				} while (cursor.moveToNext());
			}
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			if (database != null) {
				database.close();
			}
		}

		return points;
	}

	@Override
	public int compareTo(File another) {
		return Long.compare(another.lastModified(), lastModified());
	}
}
