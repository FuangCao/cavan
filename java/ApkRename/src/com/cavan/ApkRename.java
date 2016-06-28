package com.cavan;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import com.cavan.java.AndroidManifest;
import com.cavan.java.CavanCommand;
import com.cavan.java.CavanFile;
import com.cavan.java.CavanFile.CavanReplaceHandler;
import com.cavan.java.CavanJava;
import com.cavan.java.CavanXml;

public class ApkRename {

	public static final boolean RENAME_USE_DATE = false;
	public static final String KEYSTORE = "/cavan/build/core/cavan.keystore";
	public static final Path DEFAULT_WORK_PATH = Paths.get("/tmp", "cavan-apk-rename");

	private static String[] sFullRenamePackages = {
		"com.pplive.androidxl",
		"com.togic.livevideo",
		"net.myvst.v2",
		"cn.cibntv.ott",
		"com.dianlv.tv",
	};

	private static String[] sSimpleRenamePackages = {
		"com.qiyi.video",
		"cn.com.wasu.main",
		"com.elinkway.tvlive2",
		"com.ktcp.video",
		"com.letv.tv",
		"com.molitv.android",
		"com.moretv.android",
		"com.sohuott.tv.vod",
		"com.starcor.mango",
		"cn.beevideo",
		"com.sohutv.tv",
		"com.tudou.tv.c",
		"com.xunlei.kankan.tv",
		"com.fanshi.tvvideo",
	};

	private static HashMap<String, String> sHashMapImage = new HashMap<String, String>();
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
	private String mSourceDataPath;
	private String mDestDataPath;
	private String mAppNameProp;
	private boolean mFullRename;
	private AndroidManifest mAndroidManifest;
	private HashMap<String, String> mHashMapAppName = new HashMap<String, String>();

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
		} catch (Exception e) {
			e.printStackTrace();
		}

		return false;
	}

	public static boolean doApkDecode(String inPath, String outPath) {
		CavanJava.logD("decode: " + inPath + " => " + outPath);
		return doApktool("decode", inPath, outPath);
	}

	public static boolean doApkEncode(String inPath, String outPath) {
		CavanJava.logD("encode: " + inPath + " => " + outPath);
		return doApktool("build", inPath, outPath);
	}

	public static boolean doApkSign(String inPath, String outPath) {
		CavanCommand command = new CavanCommand(
				"jarsigner", "-digestalg", "SHA1", "-sigalg", "MD5withRSA",
				"-tsa", "https://timestamp.geotrust.com/tsa",
				"-storepass", "CFA8888", "-keystore", KEYSTORE,
				"-signedjar", outPath, inPath, KEYSTORE);

		CavanJava.logD("signer: " + inPath + " => " + outPath);

		return command.doExec();
	}

	public static boolean doApkAlign(String inPath, String outPath) {
		CavanCommand command = new CavanCommand("zipalign", "-v", "4", inPath, outPath);

		CavanJava.logD("align: " + inPath + " => " + outPath);

		command.closeOut();

		return command.doExec();
	}

	public boolean doRenameXml(NodeList nodeList, int depth) {
		boolean changed = false;

		for (int i = nodeList.getLength() - 1; i >= 0; i--) {
			Node node = nodeList.item(i);
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
						CavanJava.logD(name + ": " + value + " => " + newValue);
						attr.setNodeValue(newValue);
						changed = true;
					}
				}
			}

			if (depth > 1) {
				NodeList childList = element.getChildNodes();
				if (childList != null && doRenameXml(childList, depth - 1)) {
					changed = true;
				}
			}
		}

		return changed;
	}

	public boolean doRenameXml(File file) {
		try {
			CavanXml xml = new CavanXml(file);
			Document document = xml.getDocument();
			NodeList nodeList = document.getChildNodes();

			if (nodeList == null) {
				return true;
			}

			boolean changed = (mFullRename && doRenameXml(nodeList, 3));

			if (mAppNameProp != null && file.getName().equals("strings.xml")) {
				nodeList = document.getElementsByTagName("string");
				for (int i = 0; i < nodeList.getLength(); i++) {
					Node node = nodeList.item(i);
					if (node.getNodeType() != Node.ELEMENT_NODE) {
						continue;
					}

					Element element = (Element) node;
					String name = element.getAttribute("name");
					if (name.equals(mAppNameProp)) {
						Node valueNode = element.getFirstChild();
						if (valueNode == null) {
							break;
						}

						String nameValue = valueNode.getNodeValue();
						if (nameValue == null) {
							break;
						}

						nameValue = CavanJava.strStrip(nameValue);

						String type = file.getParentFile().getName();
						addAppName(type, nameValue);

						String newName = renameAppName(nameValue);
						CavanJava.logD(type + "@" + mAppNameProp + ": " + nameValue + " => " + newName);

						valueNode.setNodeValue(newName);
						changed = true;
						break;
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
				String mime = CavanFile.getMimeType(file.getPath());
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

						CavanJava.logD("rename: " + file.getPath() + " => " + pathname);
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
		line = line.replace(mSourceDataPath, mDestDataPath);

		Matcher matcher = sPatternSmaliGetIdentifier.matcher(line);
		if (matcher.find()) {
			CavanJava.logP(line);
			line = matcher.group(1) + "const-string/jumbo " + matcher.group(2) + ", \"" + mDestPackage + "\"\n" + line;
		}

		return line;
	}

	public boolean doRenameSmaliDir(File dir) {
		for (File file : dir.listFiles()) {
			if (!doRenameSmaliFile(file)) {
				return false;
			}
		}

		return true;
	}

	public boolean doRenameSmaliFile(File file) {
		if (file.isDirectory()) {
			return doRenameSmaliDir(file);
		}

		CavanFile cavanFile = new CavanFile(file.getPath());

		return cavanFile.replaceLines(new CavanReplaceHandler() {

			@Override
			public String replace(String text) {
				text = text.replace("\"" + mSourcePackage + "\"", "\"" + mDestPackage + "\"");
				return doReplaceSmaliLine(text);
			}
		});
	}

	public boolean doCopySmaliFile(CavanFile fileSource, CavanFile fileDest) {
		return fileSource.replaceLines(new CavanReplaceHandler() {

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
			CavanJava.logP("Failed to mkdirs: " + dirDest.getPath());
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

	private File buildSmaliDir(File dir, int index) {
		if (index < 2) {
			return dir;
		}

		return new File(dir.getPath() + "_classes" + index);
	}

	public boolean doRenameSmali(File dirTop) {
		List<File> dirs = new ArrayList<File>();

		dirs.add(dirTop);

		int index = 2;

		while (true) {
			File file = buildSmaliDir(dirTop, index);
			if (file.isDirectory()) {
				dirs.add(file);
			} else {
				break;
			}

			index++;
		}

		for (File dir : dirs) {
			CavanJava.logD("rename: " + dir.getPath());
			if (!doRenameSmaliDir(dir)) {
				return false;
			}

			CavanFile dirSource = new CavanFile(dir, mSourcePackagePath);
			if (!dirSource.isDirectory()) {
				continue;
			}

			CavanFile dirDest = new CavanFile(buildSmaliDir(dirTop, index), mDestPackagePath);

			CavanJava.logD("copy: " + dirSource.getPath() + " => " + dirDest.getPath());
			if (!doCopySmaliDir(dirSource, dirDest)) {
				return false;
			}

			index++;
		}

		return true;
	}

	public boolean doRenameAssetsDir(File dir) {
		for (File file : dir.listFiles()) {
			if (!doRenameAssets(file)) {
				return false;
			}
		}

		return true;
	}

	public boolean doRenameAssets(File file) {
		if (file.isDirectory()) {
			return doRenameAssetsDir(file);
		}

		CavanFile cavanFile = new CavanFile(file.getPath());
		String mime = cavanFile.getMimeType();
		if (mime == null) {
			return true;
		}

		if (mime.startsWith("text/") || mime.equals("application/xml")) {
			CavanJava.logD("rename: [" + mime + "] " + cavanFile.getPath());
			return cavanFile.replaceText(new CavanReplaceHandler() {

				@Override
				public String replace(String text) {
					return text.replace(mSourceDataPath, mDestDataPath);
				}
			});
		}

		return true;
	}

	public boolean doRenameApktool(CavanFile file) {
		CavanJava.logD("rename: " + file.getPath());

		return file.replaceLines(new CavanReplaceHandler() {

			@Override
			public String replace(String text) {
				String[] values = text.split(":\\s*");
				if (values.length == 2) {
					String key = values[0].trim();
					if (key.equals("renameManifestPackage")) {
						String value = values[1].trim();
						if (!value.equals("null")) {
							mFullRename = false;
							updateDestPackageName(value);
						}

						if (!mFullRename) {
							text = values[0] + ": " + mDestPackage;
							CavanJava.logD(mSourcePackage + " => " + mDestPackage);
						}
					}
				}

				return text;
			}
		});
	}

	public void setDestPackageName(String pkgName) {
		mDestPackage = pkgName;
		mDestDataPath = "/data/data/" + mDestPackage;
		mDestPackagePath = mDestPackage.replace('.', File.separatorChar);
	}

	public void updateDestPackageName(String pkgName) {
		if (RENAME_USE_DATE) {
			SimpleDateFormat format = new SimpleDateFormat("YMMddHHmmss");
			pkgName = "com.cavan_" + format.format(new Date()) + "." + pkgName;
		} else {
			pkgName = "com.cavan." + pkgName;
		}

		setDestPackageName(pkgName);
	}

	public void setSourcePackageName(String pkgName) {
		mSourcePackage = pkgName;
		mSourceDataPath = "/data/data/" + mSourcePackage;
		mSourcePackagePath = mSourcePackage.replace('.', File.separatorChar);

		if (mDestPackage == null) {
			updateDestPackageName(pkgName);
		}
	}

	public String renameAppName(String name) {
		return name + "-CFA";
	}

	public String addAppName(String type, String name) {
		CavanJava.logD(type + ": app_name = " + name);
		return mHashMapAppName.put(type, name);
	}

	public String getAppName() {
		if (mHashMapAppName.size() > 1) {
			String appName = mHashMapAppName.get("values-zh-rCN");
			if (appName != null) {
				return appName;
			}

			appName = mHashMapAppName.get("values");
			if (appName != null && CavanJava.hasChineseChar(appName)) {
				return appName;
			}

			for (String key : new String[] { "values-zh-rHK", "values-zh-rTW" }) {
				String name = mHashMapAppName.get(key);
				if (name != null) {
					return name;
				}
			}

			for (String key : mHashMapAppName.keySet()) {
				if (key.startsWith("values-zh")) {
					return mHashMapAppName.get(key);
				}
			}

			if (appName != null) {
				return appName;
			}
		}

		for (String value : mHashMapAppName.values()) {
			return value;
		}

		return null;
	}

	public boolean doRename(boolean fullRename) {

		mFullRename = fullRename;

		CavanJava.logD("rename: " + mInFile.getPath() + " => " + mOutFile.getPath());

		if (!CavanFile.deleteAll(mWorkFile)) {
			CavanJava.logP("Failed to deleteAll: " + mWorkFile.getPath());
			return false;
		}

		if (!doApkDecode(mInFile.getPath(), mWorkFile.getPath())) {
			CavanJava.logP("Failed to doApkDecode: " + mWorkFile.getPath());
			return false;
		}

		try {
			mAndroidManifest = new AndroidManifest(new File(mWorkFile, "AndroidManifest.xml"));
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}

		setSourcePackageName(mAndroidManifest.getPackageName());

		if (mFullRename) {
			for (String pkgName : sSimpleRenamePackages) {
				if (pkgName.equals(mSourcePackage)) {
					mFullRename = false;
					break;
				}
			}
		} else {
			for (String pkgName : sFullRenamePackages) {
				if (pkgName.equals(mSourcePackage)) {
					mFullRename = true;
					break;
				}
			}
		}

		CavanJava.logD("mFullRename = " + mFullRename);
		CavanJava.logD("package: " + mSourcePackage + " => " + mDestPackage);

		boolean manifestChanged = false;

		mAppNameProp = mAndroidManifest.getAppName();
		if (mAppNameProp != null) {
			if (mAppNameProp.startsWith("@string/")) {
				mAppNameProp = mAppNameProp.substring(8);
			} else {
				if (mAppNameProp.charAt(0) != '@') {
					addAppName("default",	mAppNameProp);
					mAndroidManifest.setAppName(renameAppName(mAppNameProp));
					manifestChanged = true;
				}

				mAppNameProp = null;
			}
		} else {
			mAppNameProp = "app_name";
		}

		if (!doRenameApktool(new CavanFile(mWorkFile, "apktool.yml"))) {
			CavanJava.logE("Failed to doRenameApktool");
			return false;
		}

		if (!doRenameResource(new File(mWorkFile, "res"))) {
			CavanJava.logE("Failed to doRenameResource");
			return false;
		}

		if (mFullRename) {
			if (!mAndroidManifest.doRename(mDestPackage)) {
				CavanJava.logE("Failed to mAndroidManifest.doRename");
				return false;
			}

			if (!doRenameSmali(new File(mWorkFile, "smali"))) {
				CavanJava.logE("Failed to doRenameSmali");
				return false;
			}

			if (!doRenameAssets(new File(mWorkFile, "assets"))) {
				CavanJava.logE("Failed to doRenameAssets");
				return false;
			}
		} else if (manifestChanged) {
			if (!mAndroidManifest.save()) {
				CavanJava.logE("Failed to mAndroidManifest.save");
				return false;
			}
		}

		mApkUnsigned.delete();

		if (!doApkEncode(mWorkFile.getPath(), mApkUnsigned.getPath())) {
			CavanJava.logE("Failed to doApkEncode");
			return false;
		}

		mApkSigned.delete();

		if (!doApkSign(mApkUnsigned.getPath(), mApkSigned.getPath())) {
			CavanJava.logE("Failed to doApkSign");
			return false;
		}

		mOutFile.delete();

		if (!doApkAlign(mApkSigned.getPath(), mOutFile.getPath())) {
			CavanJava.logE("Failed to doApkAlign");
			return false;
		}

		CavanJava.logD("File stored in: " + mOutFile.getPath());

		return true;
	}

	public boolean doRename() {
		return doRename(true);
	}
}
