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

import com.cavan.java.AndroidManifest;
import com.cavan.java.CavanCommand;
import com.cavan.java.CavanFile;
import com.cavan.java.CavanFile.ReplaceHandler;
import com.cavan.java.CavanJava;
import com.cavan.java.CavanXml;

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
	private String mAppNameProp;
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

	public static String getFileMimeType(String pathname) {
		CavanCommand command = new CavanCommand( "file", "-b", "--mime-type", pathname );
		List<String> lines = command.doPipe();
		if (lines != null && lines.size() > 0) {
			return lines.get(0);
		}

		return null;
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

			boolean changed = doRenameXml(nodeList, 3);

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

						String type = file.getParentFile().getName();
						mHashMapAppName.put(type, nameValue);

						String newName = nameValue + "-CFA";
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
		line = line.replace("/data/data/" + mSourcePackage, "/data/data/" + mDestPackage);

		Matcher matcher = sPatternSmaliGetIdentifier.matcher(line);
		if (matcher.find()) {
			CavanJava.logP(line);
			line = matcher.group(1) + "const-string/jumbo " + matcher.group(2) + ", \"" + mDestPackage + "\"\n" + line;
		}

		return line;
	}

	public boolean doRenameSmaliFile(File file) {
		CavanFile cavanFile = new CavanFile(file.getPath());
		return cavanFile.replaceLines(new ReplaceHandler() {

			@Override
			public String replace(String text) {
				if (mSourcePackage.equals("com.qiyi.video")) {
					text = text.replace("\"" + mSourcePackage + "\"", "\"" + mDestPackage + "\"");
				}

				return doReplaceSmaliLine(text);
			}
		});
	}

	public boolean doRenameSmaliDir(File dir) {
		for (File file : dir.listFiles()) {
			if (file.isDirectory()) {
				if (!doRenameSmaliDir(file)) {
					return false;
				}
			} else if (!doRenameSmaliFile(file)) {
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

	public boolean doRename() {

		CavanJava.logD("rename: " + mInFile.getPath() + " => " + mOutFile.getPath());

		if (!CavanFile.deleteAll(mWorkFile)) {
			CavanJava.logP("Failed to deleteAll: " + mWorkFile.getPath());
			return false;
		}

		if (!doApkDecode(mInFile.getPath(), mWorkFile.getPath())) {
			CavanJava.logP("Failed to doApkDecode: " + mWorkFile.getPath());
			return false;
		}

		AndroidManifest manifest;

		try {
			manifest = new AndroidManifest(new File(mWorkFile, "AndroidManifest.xml"));
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}

		mAppNameProp = manifest.getAppName();
		mSourcePackage = manifest.getPackageName();

		if (mDestPackage == null) {
			mDestPackage = "com.cavan." + mSourcePackage;
		}

		CavanJava.logD("package: " + mSourcePackage + " => " + mDestPackage);

		mSourcePackagePath = mSourcePackage.replace('.', File.separatorChar);
		mDestPackagePath = mDestPackage.replace('.', File.separatorChar);

		manifest.doRename(mDestPackage);

		mHashMapAppName.clear();

		if (!doRenameResource(new File(mWorkFile, "res"))) {
			CavanJava.logE("Failed to doRenameResource");
			return false;
		}

		if (!doRenameSmali(new File(mWorkFile, "smali"))) {
			CavanJava.logE("Failed to doRenameSmali");
			return false;
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
}
