package com.cavan.cavanjni;

import java.io.File;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;

import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanFile;


public class CavanJni extends CavanNative {
	public static final String TAG = "Cavan";

	public static native boolean kill(String name);
	public static native boolean setEnv(String key, String value);
	public static native boolean symlink(String target, String linkpath);

	public static String getEnv(String name) {
		return System.getenv(name);
	}

	public static boolean setPathEnv(String value) {
		return setEnv("PATH", value);
	}

	public static boolean appendPathEnv(String value) {
		String path = getEnv("PATH");
		if (path == null) {
			path = value;
		} else {
			path += ":" + value;
		}

		return setPathEnv(path);
	}

	public static boolean setupEnv(Context context) {
		if (context == null) {
			return false;
		}

		File cache = context.getCacheDir();
		if (cache == null) {
			return false;
		}

		CavanJni.setEnv("CACHE_PATH", cache.getPath());
		CavanJni.setEnv("HOME", cache.getParent());

		return true;
	}

	public static int doCommand(ICavanCommand command, String... args) {
		return command.main(args);
	}

	public static int doCommand(String name, String... args) {
		ICavanCommand command = sHashMap.get(name);
		if (command != null) {
			return doCommand(command, args);
		}

		return -1;
	}

	public static boolean symlinkApks(PackageManager manager, CavanFile target) {
		if (!target.mkdirsSafe()) {
			CavanAndroid.eLog("Failed to mkdirsSafe: " + target.getPath());
			return false;
		}

		target.clear();

		boolean success = true;

		for (ApplicationInfo info : manager.getInstalledApplications(0)) {
			String filename;

			CharSequence label = manager.getApplicationLabel(info);
			if (label != null) {
				filename = CavanFile.replaceInvalidFilenameChar(label.toString(), '_');
			} else {
				filename = info.packageName;
			}

			if (!symlink(info.sourceDir, new File(target, filename + ".apk").getPath())) {
				success = false;
			}
		}

		return success;
	}
}
