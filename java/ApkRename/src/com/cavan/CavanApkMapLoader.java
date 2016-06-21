package com.cavan;

import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.util.HashMap;
import java.util.Map.Entry;

import com.cavan.java.CavanFile;
import com.cavan.java.CavanFile.CavanScanHandler;

class CavanApkMapLoader extends CavanFile implements CavanScanHandler {

	private static final long serialVersionUID = 9137598415443931262L;

	private HashMap<String, String> mHashMap = new HashMap<String, String>();

	public CavanApkMapLoader(File dir, String name) {
		super(dir, name);
	}

	@Override
	public boolean scan(String line) {
		String[] values = line.split("\\s+=>\\s+");
		if (values.length > 1) {
			mHashMap.put(values[0], values[1]);
		} else {
			mHashMap.put(values[0], null);
		}

		return true;
	}

	public boolean load() {
		mHashMap.clear();

		if (exists()) {
			return scanLines(this);
		}

		return true;
	}

	public boolean hasApk(String apk) {
		return mHashMap.containsKey(apk);
	}

	public boolean addApk(String apk, String name) {
		mHashMap.put(apk, name);
		return appendText(buildLine(apk, name)) > 0;
	}

	public boolean delApk(String apk) {
		if (mHashMap.containsKey(apk)) {
			mHashMap.remove(apk);
			return save();
		}

		return true;
	}

	private String buildLine(String apk, String name) {
		StringBuilder builder = new StringBuilder(apk);
		if (name != null) {
			builder.append(" => ");
			builder.append(name);
		}

		builder.append("\r\n");

		return builder.toString();
	}

	private byte[] buildLineBytes(String apk, String name) {
		return buildLine(apk, name).getBytes();
	}

	public boolean save() {
		OutputStream stream = openOutputStream();
		if (stream == null) {
			return false;
		}

		try {
			for (Entry<String, String> entry : mHashMap.entrySet()) {
				stream.write(buildLineBytes(entry.getKey(), entry.getValue()));
			}

			return true;
		} catch (IOException e) {
			e.printStackTrace();
			return false;
		} finally {
			try {
				stream.close();
			} catch (IOException e) {
				e.printStackTrace();
				return false;
			}
		}
	}
};