package com.cavan;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import brut.common.BrutException;

import com.cavan.CavanFile.ReplaceHandler;

public class ApkRename {

	public static final String KEYSTORE = "/cavan/build/core/cavan.keystore";
	public static final Path DEFAULT_WORK_PATH = Paths.get("/tmp", "cavan-apk-rename");

	public static HashMap<String, String> sHashMapImage = new HashMap<String, String>();
	private static Pattern sPatternSmaliGetIdentifier = Pattern.compile("^(\\s*)invoke-virtual\\s*\\{\\s*\\w+,\\s*\\w+,\\s*\\w+,\\s*(\\w+)\\s*\\}\\s*,\\s*Landroid/content/res/Resources;->getIdentifier\\(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;\\)I");

	private File mWorkFile;
	private File mInFile;
	private File mOutFile;
	private File mApkSigned;
	private File mApkUnsigned;

	private String mSourcePackage;
	private String mDestPackage;
	private String mSourcePackagePath;
	private String mDestPackagePath;

	static {
		sHashMapImage.put("image/png", ".png");
		sHashMapImage.put("image/jpeg", ".jpg");
		sHashMapImage.put("image/gif", ".gif");
	}

	public ApkRename(File workFile, File inFile, File outFile) {
		if (workFile == null) {
			mWorkFile = new File(DEFAULT_WORK_PATH.toString());
		} else {
			mWorkFile = workFile;
		}

		mInFile = inFile;

		if (outFile == null) {
			mOutFile = new File(mWorkFile, "cavan.apk");
		} else {
			mOutFile = outFile;
		}

		mApkUnsigned = new File(mWorkFile, "cavan-unsigned.apk");
		mApkSigned = new File(mWorkFile, "cavan-signed.apk");
	}

	public ApkRename(File inFile, File outFile) {
		this(null, inFile, outFile);
	}

	public ApkRename(File file) {
		this(file, null);
	}

	public ApkRename(String pathname) {
		this(new File(pathname));
	}

	public void setDestPackage(String name) {
		mDestPackage = name;
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
		CavanUtils.logD("decode: " + inPath + " => " + outPath);
		return doApktool("decode", inPath, outPath);
	}

	public static boolean doApkEncode(String inPath, String outPath) {
		CavanUtils.logD("encode: " + inPath + " => " + outPath);
		return doApktool("build", inPath, outPath);
	}

	public static boolean doApkSign(String inPath, String outPath) {
		CavanCommand command = new CavanCommand(
				"jarsigner", "-digestalg", "SHA1", "-sigalg", "MD5withRSA",
				"-tsa", "https://timestamp.geotrust.com/tsa",
				"-storepass", "CFA8888", "-keystore", KEYSTORE,
				"-signedjar", outPath, inPath, KEYSTORE);

		CavanUtils.logD("signer: " + inPath + " => " + outPath);

		return command.doExec();
	}

	public static boolean doApkAlign(String inPath, String outPath) {
		CavanCommand command = new CavanCommand("zipalign", "-v", "4", inPath, outPath);

		CavanUtils.logD("align: " + inPath + " => " + outPath);

		command.closeStdout();

		return command.doExec();
	}

	public static String getFileMimeType(String pathname) {
		CavanCommand command = new CavanCommand( "file", "-b", "--mime-type", pathname );
		List<String> lines = command.doPipe();
		if (lines != null && lines.size() > 0) {
			return lines.get(0);
		}

		return null;
	}

	public boolean doRenameXml(File file) {
		try {
			CavanXml xml = new CavanXml(file);
			Document document = xml.getDocument();
			NodeList list = document.getChildNodes();
			if (list == null) {
				return false;
			}

			boolean changed = false;

			for (int i = list.getLength() - 1; i >= 0; i--) {
				Node node = list.item(i);
				if (node.getNodeType() != Node.ELEMENT_NODE) {
					continue;
				}

				Element element = (Element) node;
				NamedNodeMap map = element.getAttributes();
				if (map == null) {
					continue;
				}

				for (int j = map.getLength() - 1; j >= 0; j--) {
					Node attr = map.item(j);
					String name = attr.getNodeName();
					if (name.startsWith("xmlns:")) {
						String value = attr.getNodeValue();
						if (value.endsWith("/" + mSourcePackage)) {
							String newValue = value.substring(0, value.length() - mSourcePackage.length()) + mDestPackage;
							CavanUtils.logD(name + ": " + value + " => " + newValue);
							attr.setNodeValue(newValue);
							changed = true;
						}
					}
				}
			}

			if (changed && xml.save() == false) {
				return false;
			}

			return true;
		} catch (Exception e) {
			e.printStackTrace();
		}

		return false;
	}

	public boolean doRenameResource(File dir) {
		for (File file : dir.listFiles()) {
			if (file.isDirectory()) {
				if (!doRenameResource(file)) {
					return false;
				}
			} else {
				String mime = getFileMimeType(file.getPath());
				if (mime == null) {
					continue;
				}

				if (mime.equals("application/xml")) {
					if (!doRenameXml(file)) {
						return false;
					}
				} else {
					String image = sHashMapImage.get(mime);
					if (image != null && file.getPath().endsWith(image) == false) {
						String pathname = file.getPath();
						int index = pathname.lastIndexOf('.');
						if (index < 0) {
							pathname = pathname + image;
						} else {
							pathname = pathname.substring(0, index) + image;
						}

						CavanUtils.logD("rename: " + file.getPath() + " => " + pathname);
						if (!file.renameTo(new File(pathname))) {
							return false;
						}
					}
				}
			}
		}

		return true;
	}

	public boolean doUpdateFile(File file) {
		InputStream inStream = null;

		try {
			inStream = new FileInputStream(file);
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}

		BufferedReader reader = new BufferedReader(new InputStreamReader(inStream));
		List<String> lines = new ArrayList<String>();

		try {
			while (true) {
				String line = reader.readLine();
				if (line == null) {
					break;
				}

				lines.add(line);
			}
		} catch (IOException e) {
			e.printStackTrace();
			return false;
		} finally {
			try {
				reader.close();
				inStream.close();
			} catch (Exception e) {
				e.printStackTrace();
				return false;
			}
		}

		return true;
	}

	public String doReplaceSmaliLine(String line) {
		line = line.replace("/data/data/" + mSourcePackage, "/data/data/" + mDestPackage);

		Matcher matcher = sPatternSmaliGetIdentifier.matcher(line);
		if (matcher.find()) {
			CavanUtils.logP(line);
			line = matcher.group(1) + "const-string/jumbo " + matcher.group(2) + ", \"" + mDestPackage + "\"\n" + line;
		}

		return line;
	}

	public boolean doRenameSmaliFileNormal(File file) {
		CavanFile cavanFile = new CavanFile(file.getPath());
		return cavanFile.replaceLines(new ReplaceHandler() {

			@Override
			public String replace(String text) {
				text = text.replace("\"" + mSourcePackage + "\"", "\"" + mDestPackage + "\"");
				return doReplaceSmaliLine(text);
			}
		});
	}

	public boolean doRenameSmaliNormal(File dir) {
		for (File file : dir.listFiles()) {
			if (file.isDirectory()) {
				if (!doRenameSmaliNormal(file)) {
					return false;
				}
			} else if (!doRenameSmaliFileNormal(file)) {
				return false;
			}
		}

		return true;
	}

	public boolean doRenameSmaliFileQiyi(File file) {
		CavanFile cavanFile = new CavanFile(file.getPath());
		return cavanFile.replaceLines(new ReplaceHandler() {

			@Override
			public String replace(String text) {
				return doReplaceSmaliLine(text);
			}
		});
	}

	public boolean doRenameSmaliQiyi(File dir) {
		for (File file : dir.listFiles()) {
			if (file.isDirectory()) {
				if (!doRenameSmaliQiyi(file)) {
					return false;
				}
			} else if (!doRenameSmaliFileQiyi(file)) {
				return false;
			}
		}

		return true;
	}

	public boolean doCopySmaliFile(CavanFile fileSource, CavanFile fileDest) {
		return fileSource.replaceLines(new ReplaceHandler() {

			@Override
			public String replace(String text) {
				if (text.startsWith(".class")) {
					return text.replace("L" + mSourcePackagePath, "L" + mDestPackagePath);
				}

				return text;
			}
		}, fileDest);
	}

	public boolean doCopySmaliDir(CavanFile dirSource, CavanFile dirDest) {
		if (!dirDest.mkdirs()) {
			CavanUtils.logP("Failed to mkdirs: " + dirDest.getPath());
			return false;
		}

		for (String filename : dirSource.list()) {
			CavanFile fileSource = new CavanFile(dirSource, filename);
			CavanFile fileDest = new CavanFile(dirDest, filename);

			if (fileSource.isDirectory()) {
				if (!doCopySmaliDir(fileSource, fileDest)) {
					return false;
				}
			} else if (!doCopySmaliFile(fileSource, fileDest)) {
				return false;
			}
		}

		return true;
	}

	public boolean doCopySmali(File dir) {
		CavanFile dirSource = new CavanFile(dir, mSourcePackagePath);
		if (!dirSource.isDirectory()) {
			return true;
		}

		dir = new File(dir.getPath() + "_classes2");
		CavanFile dirDest = new CavanFile(dir, mDestPackagePath);

		return doCopySmaliDir(dirSource, dirDest);
	}

	public boolean doRenameSmali(File dir) {
		boolean success;

		if (mSourcePackage.equals("com.qiyi.video")) {
			success = doRenameSmaliQiyi(dir);
		} else {
			success = doRenameSmaliNormal(dir);
		}

		if (success) {
			success = doCopySmali(dir);
		}

		return success;
	}

	public boolean doRename() {

		CavanUtils.logD("rename: " + mInFile.getPath() + " => " + mOutFile.getPath());

		if (!CavanFile.deleteAll(mWorkFile)) {
			CavanUtils.logP("Failed to deleteAll: " + mWorkFile.getPath());
			return false;
		}

		if (!doApkDecode(mInFile.getPath(), mWorkFile.getPath())) {
			CavanUtils.logP("Failed to doApkDecode: " + mWorkFile.getPath());
			return false;
		}

		AndroidManifest manifest;

		try {
			manifest = new AndroidManifest(new File(mWorkFile, "AndroidManifest.xml"));
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}

		mSourcePackage = manifest.getPackageName();

		if (mDestPackage == null) {
			mDestPackage = "com.cavan." + mSourcePackage;
		}

		CavanUtils.logD("package: " + mSourcePackage + " => " + mDestPackage);

		mSourcePackagePath = mSourcePackage.replace('.', File.separatorChar);
		mDestPackagePath = mDestPackage.replace('.', File.separatorChar);

		manifest.doRename(mDestPackage);

		if (!doRenameResource(new File(mWorkFile, "res"))) {
			CavanUtils.logE("Failed to doRenameResource");
			return false;
		}

		if (!doRenameSmali(new File(mWorkFile, "smali"))) {
			CavanUtils.logE("Failed to doRenameSmali");
			return false;
		}

		if (!doApkEncode(mWorkFile.getPath(), mApkUnsigned.getPath())) {
			CavanUtils.logE("Failed to doApkEncode");
			return false;
		}

		if (!doApkSign(mApkUnsigned.getPath(), mApkSigned.getPath())) {
			CavanUtils.logE("Failed to doApkSign");
			return false;
		}

		if (!doApkAlign(mApkSigned.getPath(), mOutFile.getPath())) {
			CavanUtils.logE("Failed to doApkAlign");
			return false;
		}

		CavanUtils.logD("File stored in: " + mOutFile.getPath());

		return true;
	}

	public static boolean doRenameAll(File inFile, File outDir) {
		if (inFile.isDirectory()) {
			for (File file : inFile.listFiles()) {
				if (!doRenameAll(file, outDir)) {
					CavanUtils.logP("Failed to doRenameAll: " + file.getPath());
					return false;
				}
			}
		} else {
			CavanUtils.printSep();

			String outName = inFile.getName().replaceAll("\\.apk$", "-cavan.apk");
			CavanFile outFile = new CavanFile(outDir, outName);
			if (outFile.exists()) {
				CavanUtils.logD("skip exists file: " + outFile.getPath());
				return true;
			}

			ApkRename rename = new ApkRename(inFile, outFile);
			if (!rename.doRename()) {
				CavanUtils.logP("Failed to doRename: " + inFile.getPath());

				CavanFile errDir = new CavanFile(outDir, "failure");
				if (!errDir.mkdirSafe()) {
					CavanUtils.logP("Failed to mkdirSafe: " + errDir.getPath());
					return false;
				}

				CavanFile errFile = new CavanFile(errDir, inFile.getName());
				if (!errFile.copyFrom(inFile)) {
					CavanUtils.logP("Failed to copy: " + errFile.getPath());
					return false;
				}
			}
		}

		return true;
	}

	public static void main(String[] args) throws Exception {
		boolean success = false;

		if (args.length > 1) {
			int	count = args.length - 1;
			CavanFile outDir = new CavanFile(args[count]);

			if (outDir.mkdirsSafe()) {
				for (int i = 0; i < count; i++) {
					success = doRenameAll(new File(args[i]), outDir);
					if (!success) {
						break;
					}
				}
			}
		} else if (args.length > 0) {
			ApkRename rename = new ApkRename(args[0]);
			success = rename.doRename();
		} else {
			CavanUtils.logD("apkrename <IN_APK> ... [OUT_APK]");
		}

		if (!success) {
			throw new Exception("Failed to ApkRename");
		}
	}
}
