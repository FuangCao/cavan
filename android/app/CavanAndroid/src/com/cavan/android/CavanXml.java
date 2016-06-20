package com.cavan.android;

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
import org.xml.sax.SAXException;

public class CavanXml {

	protected File mFileDocument;
	protected Document mDocument;

	public CavanXml(File file) throws ParserConfigurationException, SAXException, IOException {
		super();

		mFileDocument = file;

		DocumentBuilder builder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
		mDocument = builder.parse(mFileDocument);
	}

	public Document getDocument() {
		return mDocument;
	}

	public static void replaceAttribute(Element element, String name, String value) {
		element.removeAttribute(name);
		element.setAttribute(name, value);
	}

	public boolean save(File file) {
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

	public boolean save(String pathname) {
		return save(new File(pathname));
	}

	public boolean save() {
		return save(mFileDocument);
	}
}
