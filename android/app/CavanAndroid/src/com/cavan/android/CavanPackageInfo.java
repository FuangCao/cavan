package com.cavan.android;

import java.io.File;

import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;

import com.cavan.java.CavanFile;

public class CavanPackageInfo {

	protected String mName;
	protected PackageInfo mPackageInfo;
	protected PackageManager mPackageManager;

	public CavanPackageInfo(PackageManager manager, PackageInfo info) {
		mPackageManager = manager;
		mPackageInfo = info;

		CharSequence label = manager.getApplicationLabel(info.applicationInfo);
		if (label == null) {
			mName = info.packageName;
		} else {
			mName = label.toString();
		}
	}

	public PackageManager getPackageManager() {
		return mPackageManager;
	}

	public PackageInfo getPackageInfo() {
		return mPackageInfo;
	}

	public ApplicationInfo getApplicationInfo() {
		return mPackageInfo.applicationInfo;
	}

	public String getApplicationName() {
		return mName;
	}

	public String getPackageName() {
		return mPackageInfo.packageName;
	}

	public int getVersionCode() {
		return mPackageInfo.versionCode;
	}

	public String getVersionName() {
		return mPackageInfo.versionName;
	}

	public String getSourcePath() {
		return getApplicationInfo().sourceDir;
	}

	public File getSourceFile() {
		return new File(getSourcePath());
	}

	public static boolean isGameApp(ApplicationInfo info) {
		if ((info.flags & ApplicationInfo.FLAG_IS_GAME) != 0) {
			return true;
		}

		if (info.packageName.startsWith("com.tencent.tmgp.")) {
			return true;
		}

		if (info.packageName.contains("game")) {
			return true;
		}

		return false;
	}

	public boolean isGameApp() {
		return isGameApp(mPackageInfo.applicationInfo);
	}

	public String getBackupName() {
		StringBuilder builder = new StringBuilder();

		builder.append(CavanFile.replaceInvalidFilenameChar(mName, '_'));
		builder.append('-');
		builder.append(getVersionName());
		builder.append(".apk");

		return builder.toString();
	}

	public int getFlags() {
		return getApplicationInfo().flags;
	}

	public boolean isSystemApp() {
		return (getFlags() & ApplicationInfo.FLAG_SYSTEM) != 0;
	}
}