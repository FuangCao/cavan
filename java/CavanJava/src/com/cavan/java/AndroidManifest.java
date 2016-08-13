package com.cavan.java;

import java.io.File;
import java.io.IOException;

import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

public class AndroidManifest extends CavanXml {

	private Element mManifest;
	private Element mApplication;
	private String mSourcePackage;
	private String mDestPackage;
	private String mAppNameAttr = "android:label";

	public AndroidManifest(File file) throws ParserConfigurationException, SAXException, IOException {
		super(file);

		mManifest = mDocument.getDocumentElement();

		NodeList nodes = mManifest.getElementsByTagName("application");
		if (nodes != null && nodes.getLength() > 0) {
			mApplication = (Element) nodes.item(0);
		}

		mSourcePackage = getPackageName();
		mDestPackage = mSourcePackage;
	}

	private void renameAndroidName(Element element) {
		String value = element.getAttribute("android:name");
		if (value != null && value.startsWith(".")) {
			replaceAttribute(element, "android:name", mSourcePackage + value);
		}

		value = element.getAttribute("android:authorities");
		if (value != null && value.startsWith(mSourcePackage)) {
			Element newElement = (Element) element.cloneNode(true);
			replaceAttribute(newElement, "android:authorities", mDestPackage + value.substring(mSourcePackage.length()));
			mApplication.appendChild(newElement);
		}
	}

	private void renameApplication() {
		if (mApplication == null) {
			return;
		}

		renameAndroidName(mApplication);

		NodeList nodes = mApplication.getChildNodes();
		if (nodes == null) {
			return;
		}

		for (int i = nodes.getLength() - 1; i >= 0; i--) {
			Node node = nodes.item(i);
			if (node.getNodeType() == Node.ELEMENT_NODE) {
				renameAndroidName((Element) node);
			}
		}
	}

	public String getPackageName() {
		return mManifest.getAttribute("package");
	}

	public void setPackageName(String name) {
		replaceAttribute(mManifest, "package", name);
	}

	public void setAppNameAttr(String attr) {
		mAppNameAttr = attr;
	}

	public String findAppName() {
		NamedNodeMap map = mApplication.getAttributes();
		for (int i = map.getLength() - 1; i >= 0; i--) {
			Node node = map.item(i);
			String name = node.getNodeName();
			if (name.endsWith(":label")) {
				setAppNameAttr(name);
				return node.getNodeValue();
			}
		}

		return null;
	}

	public String getAppName() {
		String name = mApplication.getAttribute(mAppNameAttr);
		if (name == null || name.isEmpty()) {
			name = findAppName();
			if (name == null) {
				return null;
			}
		}

		return CavanString.strip(name);
	}

	public void setAppName(String name) {
		replaceAttribute(mApplication, mAppNameAttr, name);
	}

	public boolean doRename(String name) {
		mDestPackage = name;

		setPackageName(name);
		renameApplication();

		return save();
	}
}
