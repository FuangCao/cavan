package com.cavan;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

import brut.common.BrutException;

public class ApkRename {

	public static final String KEYSTORE = "/cavan/build/core/cavan.keystore";
	public static final Path DEFAULT_WORK_PATH = Paths.get("/tmp", "cavan-apk-rename");

	public static HashMap<String, String> sHashMapImage = new HashMap<String, String>();

	private String mInPath;
	private String mOutPath;
	private String mApkSigned;
	private String mWorkPath;
	private String mApkUnsigned;

	static {
		sHashMapImage.put("image/png", ".png");
		sHashMapImage.put("image/jpeg", ".jpg");
		sHashMapImage.put("image/gif", ".gif");
	}

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
			builder.redirectError(new File("/dev/stderr"));
			builder.redirectOutput(new File("/dev/stdout"));
			Process process = builder.start();
			return process.waitFor() == 0;
		} catch (Exception e) {
			e.printStackTrace();
		}

		return false;
	}

	public static List<String> pipeCommand(String[] command) {
		try {
			ProcessBuilder builder = new ProcessBuilder(command);
			builder.redirectError(new File("/dev/stderr"));
			Process process = builder.start();

			BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
			List<String> lines = new ArrayList<String>();

			while (true) {
				String line = reader.readLine();
				if (line == null) {
					break;
				}

				lines.add(line);
			}

			reader.close();

			if (process.waitFor() != 0) {
				return null;
			}

			return lines;
		} catch (Exception e) {
			e.printStackTrace();
		}

		return null;
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

	public static String getFileMimeType(String pathname) {
		String[] command = { "file", "-b", "--mime-type", pathname };
		List<String> lines = pipeCommand(command);
		if (lines != null && lines.size() > 0) {
			return lines.get(0);
		}

		return null;
	}

	public static boolean doRenameXml(File file) {
		System.out.println("rename xml: " + file.getPath());
		try {
			CavanXml xml = new CavanXml(file);
			Document document = xml.getDocument();
			NodeList list = document.getChildNodes();
			if (list == null) {
				return false;
			}

			for (int i = list.getLength() - 1; i >= 0; i--) {
				Node node = list.item(i);
				if (node.getNodeType() != Node.ELEMENT_NODE) {
					continue;
				}

				Element element = (Element) node;
				System.out.println("element = " + element.getNodeName());
			}
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

						System.out.println("rename: " + file.getPath() + " => " + pathname);
						if (!file.renameTo(new File(pathname))) {
							return false;
						}
					}
				}
			}
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
			System.err.println("Failed to doRenameResource");
			return false;
		}

		if (!doApkEncode(mWorkPath, mApkUnsigned)) {
			System.err.println("Failed to doApkEncode");
			return false;
		}

		if (!doApkSign(mApkUnsigned, mApkSigned)) {
			System.err.println("Failed to doApkSign");
			return false;
		}

		if (!doApkAlign(mApkSigned, mOutPath)) {
			System.err.println("Failed to doApkAlign");
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
