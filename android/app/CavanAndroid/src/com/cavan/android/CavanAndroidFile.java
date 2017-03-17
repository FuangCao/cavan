package com.cavan.android;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.net.URI;

import android.content.Context;
import android.content.res.AssetManager;
import android.content.res.Resources;

import com.cavan.java.CavanFile;

public class CavanAndroidFile extends CavanFile {

	private static final long serialVersionUID = 7311307210515609537L;

	public CavanAndroidFile(File dir, String name) {
		super(dir, name);
	}

	public CavanAndroidFile(String dirPath, String name) {
		super(dirPath, name);
	}

	public CavanAndroidFile(String path) {
		super(path);
	}

	public CavanAndroidFile(URI uri) {
		super(uri);
	}

	public boolean copyFrom(AssetManager assets, String filename) {
		InputStream stream = null;

		try {
			stream = assets.open(filename);
			return copyFrom(stream);
		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			if (stream != null) {
				try {
					stream.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}

		return false;
	}

	public boolean copyFromAssets(Context context, String filename) {
		return copyFrom(context.getAssets(), filename);
	}

	public boolean copyFrom(Resources resources, int id) {
		InputStream stream = resources.openRawResource(id);
		if (stream == null) {
			return false;
		}

		boolean success = copyFrom(stream);

		try {
			stream.close();
		} catch (IOException e) {
			e.printStackTrace();
		}

		return success;
	}
}
