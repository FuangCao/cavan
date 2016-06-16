package com.cavan;

import java.io.File;
import java.io.FilenameFilter;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;

import brut.common.BrutException;

public class ApkRename {

	public static final String KEYSTORE = "/cavan/build/core/cavan.keystore";
	public static final Path DEFAULT_WORK_PATH = Paths.get("/tmp", "cavan-apk-rename");

	private String mInPath;
	private String mOutPath;
	private String mApkSigned;
	private String mWorkPath;
	private String mApkUnsigned;

	public ApkRename(String workPath, String inPath, String outPath) {
		if (workPath == null) {
			workPath = DEFAULT_WORK_PATH.toString();
		}

		if (outPath == null) {
			outPath = Paths.get(workPath, "cavan.apk").toString();
		}

		mWorkPath = workPath;
		mInPath = inPath;
		mOutPath = outPath;

		System.out.println("mWorkPath = " + mWorkPath);
		System.out.println("mInPath = " + mInPath);
		System.out.println("mOutPath = " + mOutPath);

		mApkUnsigned = Paths.get(mWorkPath, "cavan-unsigned.apk").toString();
		mApkSigned = Paths.get(mWorkPath, "cavan-signed.apk").toString();

		System.out.println("mApkUnsigned = " + mApkUnsigned);
		System.out.println("mApkSigned = " + mApkSigned);
	}

	public static boolean runCommand(String[] command) {
		try {
			ProcessBuilder builder = new ProcessBuilder(command);
			builder.redirectErrorStream();
			Process process = builder.start();

			InputStream stream = process.getInputStream();
			if (stream != null) {
				byte[] bytes = new byte[32];

				while (true) {
					int length = stream.read(bytes);
					if (length < 0) {
						break;
					}

					System.out.write(bytes, 0, length);
				}
			}

			return process.waitFor() == 0;
		} catch (Exception e) {
			e.printStackTrace();
		}

		return false;
	}

	public static boolean doApktool(String command, String inPath, String outPath) {
		ArrayList<String> args = new ArrayList<String>();

		args.add(command);
		args.add("-f");

		if (outPath != null) {
			args.add("-o");
			args.add(outPath);
		}

		args.add(inPath);

		try {
			brut.apktool.Main.main(args.toArray(new String[args.size()]));
			return true;
		} catch (IOException e) {
			e.printStackTrace();
		} catch (InterruptedException e) {
			e.printStackTrace();
		} catch (BrutException e) {
			e.printStackTrace();
		}

		return false;
	}

	public static boolean doApkDecode(String inPath, String outPath) {
		System.out.println("decode: " + inPath + " => " + outPath);
		return doApktool("decode", inPath, outPath);
	}

	public static boolean doApkEncode(String inPath, String outPath) {
		System.out.println("encode: " + inPath + " => " + outPath);
		return doApktool("build", inPath, outPath);
	}

	public static boolean doApkSign(String inPath, String outPath) {
		String[] command = { "jarsigner", "-digestalg", "SHA1", "-sigalg", "MD5withRSA", "-tsa", "https://timestamp.geotrust.com/tsa", "-storepass", "CFA8888", "-keystore", KEYSTORE, "-signedjar", outPath, inPath, KEYSTORE };

		System.out.println("sign: " + inPath + " => " + outPath);
		return runCommand(command);
	}

	public static boolean doApkAlign(String inPath, String outPath) {
		String[] command = { "zipalign", "-v", "4", inPath, outPath };

		System.out.println("align: " + inPath + " => " + outPath);
		return runCommand(command);
	}

	public static void renameXml() {

	}

	public boolean doRenameResource(File dir) {
		FilenameFilter filter = new FilenameFilter() {

			@Override
			public boolean accept(File dir, String name) {
				return name.endsWith(".xml");
			}
		};

		for (File file : dir.listFiles(filter)) {
			System.out.println("file = " + file.getPath());
		}

		return true;
	}

	public boolean doRename() {

		if (!doApkDecode(mInPath, mWorkPath)) {
			return false;
		}

		AndroidManifest manifest;

		try {
			manifest = new AndroidManifest(Paths.get(mWorkPath, "AndroidManifest.xml").toFile());
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}

		String sourcePackage = manifest.getPackageName();
		String destPackage = "com.cavan." + sourcePackage;

		System.out.println(sourcePackage + " => " + destPackage);
		manifest.doRename(destPackage);

		if (!doRenameResource(Paths.get(mWorkPath, "res").toFile())) {
			return false;
		}

		if (!doApkEncode(mWorkPath, mApkUnsigned)) {
			return false;
		}

		if (!doApkSign(mApkUnsigned, mApkSigned)) {
			return false;
		}

		if (!doApkAlign(mApkSigned, mOutPath)) {
			return false;
		}

		System.out.println("File stored in: " + mOutPath);

		return true;
	}

	public static void main(String[] args) throws Exception {
		ApkRename rename = new ApkRename(null, "/epan/apk/dangbei/dsm-3.0.6-dangbei.apk", null);
		if (!rename.doRename()) {
			throw new Exception("Failed to ApkRename");
		}
	}
}
