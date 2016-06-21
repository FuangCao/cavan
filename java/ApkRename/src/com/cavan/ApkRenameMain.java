package com.cavan;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import com.cavan.java.CavanFile;
import com.cavan.java.CavanJava;

public class ApkRenameMain {

	private CavanFile mDirOut;
	private CavanFile mDirFailure;
	private CavanFile mDirSuccessfull;
	private List<String> mInApks = new ArrayList<String>();

	public ApkRenameMain(CavanFile inFile, CavanFile outFile) {
		mDirOut = outFile;
		mDirFailure = new CavanFile(mDirOut, "failure");
		mDirSuccessfull = new CavanFile(mDirOut, "successfull");

		addApk(inFile);
	}

	public ApkRenameMain(String inPath, CavanFile outFile) {
		this(new CavanFile(inPath), outFile);
	}

	public void addApk(File file) {
		if (file.isDirectory()) {
			addApkDir(file);
		} else {
			mInApks.add(file.getPath());
		}
	}

	private void addApkDir(File dir) {
		for (File file : dir.listFiles()) {
			addApk(file);
		}
	}

	public String buildApkName(String filename, String appName) {
		for (char c : new char[] { '-', '_', ' ' }) {
			int index = filename.indexOf(c);
			if (index > 0) {
				return appName + filename.substring(index);
			}
		}

		return appName + ".apk";
	}

	public boolean doRenameFile(File inFile) {
		String filename = inFile.getName();

		CavanFile okFile = new CavanFile(mDirSuccessfull, filename);
		if (okFile.exists()) {
			CavanJava.logD("skip exists file: " + okFile.getPath());
			return true;
		}

		CavanFile errFile = new CavanFile(mDirFailure, filename);
		CavanFile outFile = new CavanFile(mDirOut, filename);
		ApkRename rename = new ApkRename(inFile, outFile);
		if (rename.doRename()) {
			String appName = rename.getAppName();
			if (appName != null) {
				String apkName = buildApkName(filename, appName.trim());
				if (!apkName.equals(filename)) {
					CavanFile namedFile = new CavanFile(mDirOut, apkName);
					if (namedFile.exists()) {
						CavanJava.logP("file exists: " + namedFile.getPath());
					} else {
						CavanJava.logD("move: " + outFile.getPath() + " => " + namedFile.getPath());
						if (!outFile.renameTo(namedFile)) {
							CavanJava.logP("Failed to renameTo: " + namedFile.getPath());
							return false;
						}

						outFile = namedFile;
					}
				}
			}

			if (okFile.writeText("apk: " + inFile.getAbsolutePath() + " => " + outFile.getAbsolutePath() + "\n") < 0) {
				CavanJava.logP("Failed to writeText: " + okFile.getPath());
				return false;
			}

			errFile.delete();
		} else {
			CavanJava.logP("Failed to doRename: " + inFile.getPath());

			if (errFile.writeText("apk: " + inFile.getAbsolutePath() + "\n") < 0) {
				CavanJava.logP("Failed to writeText: " + errFile.getPath());
				return false;
			}
		}

		return true;
	}

	public boolean doRenameDir(File inDir) {
		for (File file : inDir.listFiles()) {
			if (file.isDirectory()) {
				if (!doRenameDir(file)) {
					return false;
				}
			} else if (!doRenameFile(file)) {
				return false;
			}
		}

		return true;
	}

	public boolean doRename() {
		if (!mDirOut.mkdirsSafe()) {
			CavanJava.logP("Failed to mkdirsSafe: " + mDirOut.getPath());
			return false;
		}

		if (!mDirSuccessfull.mkdirsSafe()) {
			CavanJava.logP("Failed to mkdirsSafe: " + mDirSuccessfull.getPath());
			return false;
		}

		if (!mDirFailure.mkdirsSafe()) {
			CavanJava.logP("Failed to mkdirsSafe: " + mDirFailure.getPath());
			return false;
		}

		int count = 1;
		int error = 0;

		for (String pathname : mInApks) {
			CavanJava.logD("rename file: " + pathname + " (" + count + "/" + mInApks.size() + ")");

			File file = new File(pathname);
			if (!doRenameFile(file)) {
				CavanJava.logE("Failed to rename file: " + file.getAbsolutePath());
				error++;
			}

			CavanJava.printSep();
			count++;
		}

		if (error > 0) {
			CavanJava.logE("rename failure count = " + error);
			return false;
		}

		CavanJava.logD("rename successfull");

		return true;
	}

	public static void main(String[] args) throws Exception {
		boolean success = false;

		if (args.length > 1) {
			int	count = args.length - 1;
			CavanFile outDir = new CavanFile(args[count]);

			for (int i = 0; i < count; i++) {
				ApkRenameMain rename = new ApkRenameMain(args[i], outDir);
				success = rename.doRename();
				if (!success) {
					break;
				}
			}
		} else if (args.length > 0) {
			ApkRename rename = new ApkRename(args[0]);
			success = rename.doRename();
		} else {
			CavanJava.logD("apkrename <IN_APK> ... [OUT_APK]");
		}

		if (!success) {
			throw new Exception("Failed to ApkRename");
		}
	}
}
