package com.cavan.android;

import java.io.File;
import java.util.HashMap;
import java.util.List;

import com.cavan.java.CavanFile;

public class PowerSupply {

	private File mRootDir;

	public PowerSupply(File dir) {
		super();
		mRootDir = dir;
	}

	public PowerSupply(String pathname) {
		this(new File(pathname));
	}

	public String readType() {
		CavanFile file = new CavanFile(mRootDir, "type");
		return file.readText();
	}

	public List<String> readUevent() {
		CavanFile file = new CavanFile(mRootDir, "uevent");
		return file.readLines(null);
	}

	public HashMap<String, String> readProps() {
		List<String> lines = readUevent();
		if (lines == null) {
			return null;
		}

		HashMap<String, String> map = new HashMap<String, String>();

		for (String line : lines) {
			String[] values = line.split("\\s*=\\s*");
			if (values.length == 2) {
				map.put(values[0], values[1]);
			}
		}

		return map;
	}
}
