package com.cavan;

import java.io.File;

import com.cavan.java.CavanFile;
import com.cavan.java.CavanJava;

public class ApkRenameMain {

	private CavanFile mFileIn;
	private CavanFile mFileOut;
	private CavanFile mFileFailure;
	private CavanFile mFileSuccessfull;

	public ApkRenameMain(CavanFile inFile, CavanFile outFile) {
		mFileIn = inFile;
		mFileOut = outFile;
		mFileFailure = new CavanFile(mFileOut, "failure");
		mFileSuccessfull = new CavanFile(mFileOut, "successfull");
	}

	public ApkRenameMain(String inPath, CavanFile outFile) {
		this(new CavanFile(inPath), outFile);
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
		CavanJava.printSep();

		String filename = inFile.getName();

		CavanFile okFile = new CavanFile(mFileSuccessfull, filename);
		if (okFile.exists()) {
			CavanJava.logD("skip exists file: " + okFile.getPath());
			return true;
		}

		CavanFile errFile = new CavanFile(mFileFailure, filename);
		CavanFile outFile = new CavanFile(mFileOut, filename);
		ApkRename rename = new ApkRename(inFile, outFile);
		if (rename.doRename()) {
			String appName = rename.getAppName();
			if (appName != null) {
				String apkName = buildApkName(filename, appName.trim());
				if (!apkName.equals(filename)) {
					CavanFile namedFile = new CavanFile(mFileOut, apkName);
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
		if (!mFileOut.mkdirsSafe()) {
			CavanJava.logP("Failed to mkdirsSafe: " + mFileOut.getPath());
			return false;
		}

		if (!mFileSuccessfull.mkdirsSafe()) {
			CavanJava.logP("Failed to mkdirsSafe: " + mFileSuccessfull.getPath());
			return false;
		}

		if (!mFileFailure.mkdirsSafe()) {
			CavanJava.logP("Failed to mkdirsSafe: " + mFileFailure.getPath());
			return false;
		}

		if (mFileIn.isDirectory()) {
			return doRenameDir(mFileIn);
		} else {
			return doRenameFile(mFileIn);
		}
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
