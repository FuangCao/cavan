package com.cavan;

import java.io.File;
import java.io.IOException;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

public class AndroidManifest {

	private File mFileDocument;
	private Document mDocument;
	private Element mManifest;
	private Element mApplication;
	private String mSourcePackage;
	private String mDestPackage;

	public AndroidManifest(File file) throws ParserConfigurationException, SAXException, IOException {
		super();

		mFileDocument = file;

		DocumentBuilder builder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
		mDocument = builder.parse(mFileDocument);
		mManifest = mDocument.getDocumentElement();

		NodeList nodes = mManifest.getElementsByTagName("application");
		if (nodes != null && nodes.getLength() > 0) {
			mApplication = (Element) nodes.item(0);
		}

		System.out.println("mDocument = " + mDocument.getDocumentURI());
		System.out.println("mManifest = " + mManifest.getNodeName());
		System.out.println("mApplication = " + mApplication.getNodeName());

		mSourcePackage = getPackageName();
		mDestPackage = mSourcePackage;
		System.out.println("mSourcePackage = " + mSourcePackage);
	}

	private void replaceAttribute(Element element, String name, String value) {
		element.removeAttribute(name);
		element.setAttribute(name, value);
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

	public boolean save(String pathname) {
		File file = pathname != null ? new File(pathname) : mFileDocument;

		try {
			Transformer transformer = TransformerFactory.newInstance().newTransformer();
			DOMSource source = new DOMSource(mDocument.getFirstChild());
			StreamResult result = new StreamResult(file);
			transformer.transform(source, result);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
		}

		return false;
	}

	public boolean doRename(String name) {
		mDestPackage = name;

		setPackageName(name);
		renameApplication();

		return save(null);
	}
}
