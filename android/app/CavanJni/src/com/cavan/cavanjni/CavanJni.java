package com.cavan.cavanjni;

import java.io.File;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageInfo;
import com.cavan.java.CavanFile;


public class CavanJni extends CavanNative {
	public static final String TAG = "Cavan";

	public static native int kill(int pid);
	public static native int waitpid(int pid);
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

	public static int doCommand(CavanNativeCommand command, boolean async, String... args) {
		return command.main(async, args);
	}

	public static int doCommand(String name, boolean async, String... args) {
		CavanNativeCommand command = sHashMap.get(name);
		if (command != null) {
			return doCommand(command, async, args);
		}

		return -1;
	}

	public static CavanFile symlinkApk(CavanFile dir, CavanPackageInfo info) {
		CavanFile file = new CavanFile(dir, info.getBackupName().replaceAll("\\s+", "_"));
		if (symlink(info.getSourcePath(), file.getPath())) {
			return file;
		}

		return null;
	}

	public static boolean symlinkApks(PackageManager manager, CavanFile target) {
		if (!target.mkdirsSafe()) {
			CavanAndroid.eLog("Failed to mkdirsSafe: " + target.getPath());
			return false;
		}

		target.clear();

		boolean success = true;

		for (PackageInfo app : manager.getInstalledPackages(0)) {
			CavanPackageInfo info = new CavanPackageInfo(manager, app);
			if (symlinkApk(target, info) == null) {
				success = false;
			}
		}

		return success;
	}
}
