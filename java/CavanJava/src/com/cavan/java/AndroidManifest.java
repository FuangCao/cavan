package com.cavan.java;

import java.io.File;
import java.io.IOException;

import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

public class AndroidManifest extends CavanXml {

	private Element mManifest;
	private Element mApplication;
	private String mSourcePackage;
	private String mDestPackage;

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

	public String getAppName() {
		String name = mApplication.getAttribute("android:label");
		if (name != null && name.startsWith("@string/")) {
			return name.substring(8);
		}

		return null;
	}

	public boolean doRename(String name) {
		mDestPackage = name;

		setPackageName(name);
		renameApplication();

		return save();
	}
}
