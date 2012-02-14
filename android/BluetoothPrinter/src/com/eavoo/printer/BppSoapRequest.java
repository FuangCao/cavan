package com.eavoo.printer;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.HashMap;

import javax.obex.ClientOperation;
import javax.obex.ClientSession;
import javax.obex.HeaderSet;
import javax.obex.ResponseCodes;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import android.content.Context;
import android.os.Process;
import android.util.Log;

public class BppSoapRequest
{
	private static final String TAG = "CavanBppSoapRequest";
	private String mAction;
	private String mBody;
	private String mHttpHeader;
	private byte[] mResponse;
	private ClientSession mObexClientSession;
	private Document mDocument;
	private Element mElementBody;
	private Element mElementEnvelope;

	public String getAction()
	{
		return mAction;
	}

	public void setAction(String mAction)
	{
		this.mAction = mAction;
	}

	public String getHttpHeader()
	{
		return mHttpHeader;
	}

	public void setHttpHeader(String httpHeader)
	{
		this.mHttpHeader = httpHeader;
	}

	public String getBody()
	{
		return mBody;
	}

	public void setBody(String body)
	{
		this.mBody = body;
	}

	public BppSoapRequest(ClientSession session, String action, String body)
	{
		this.mObexClientSession = session;
		this.mAction = action;
		this.mBody = body;
		this.mHttpHeader = null;
	}

	public BppSoapRequest(ClientSession session, String action)
	{
		this(session, action, null);
	}

	public byte[] toByteArray()
	{
		StringBuilder builderBody = new StringBuilder();
		builderBody.append("<s:Envelope\r\n");
		builderBody.append("xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"\r\n");
		builderBody.append("s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n");
		builderBody.append("<s:Body>\r\n");
		builderBody.append("<u:" + mAction +" xmlns:u=\"urn:schemas-bluetooth-org:service:Printer:1\">\r\n");
		if (mBody != null)
		{
			builderBody.append(mBody + "\r\n");
		}
		builderBody.append("</u:" + mAction + ">\r\n");
		builderBody.append("</s:Body>\r\n");
		builderBody.append("</s:Envelope>");

		StringBuilder builderHeader = new StringBuilder();
		builderHeader.append("CONTENT-LENGTH: " + builderBody.length() + "\r\n");
		if (mHttpHeader != null)
		{
			builderHeader.append(mHttpHeader + "\r\n");
		}
		builderHeader.append("CONTENT-TYPE: text/xml; charset=\"utf-8\"\r\n");
		builderHeader.append("CONTENT-LANGUAGE: en-US\r\n");
		builderHeader.append("SOAPACTION: \"urn:schemas-bluetooth-org:service:Printer:1#" + mAction + "\"");

		String soapContent = builderHeader.toString() + "\r\n\r\n" + builderBody.toString();

		Log.v(TAG, "Soap Content = \n" + soapContent);

		return soapContent.getBytes();
	}

	public boolean SendToPrinter() throws IOException
	{
		HeaderSet reqHeaderSet = new HeaderSet();

		reqHeaderSet.setHeader(HeaderSet.TYPE, "x-obex/bt-SOAP");
		ClientOperation clientOperation = (ClientOperation) mObexClientSession.get(reqHeaderSet);

		Log.v(TAG, "Get operation complete");

		OutputStream outputStream = clientOperation.openOutputStream();
		Log.v(TAG, "Open OutputStream complete");

		outputStream.write(toByteArray());
		Log.v(TAG, "Write data complete");
		outputStream.close();

		int responseCode = clientOperation.getResponseCode();

		if (responseCode != ResponseCodes.OBEX_HTTP_CONTINUE && responseCode != ResponseCodes.OBEX_HTTP_OK)
		{
			Log.v(TAG, "responseCode != ResponseCodes.OBEX_HTTP_CONTINUE && responseCode != ResponseCodes.OBEX_HTTP_OK");
			return false;
		}

		InputStream inputStream = clientOperation.openInputStream();
		Log.v(TAG, "Open InputStream complete");

		long length = clientOperation.getLength();
		Log.v(TAG, "length = " + length);

		mResponse = new byte[(int) length];

		inputStream.read(mResponse);
		inputStream.close();

		Log.v(TAG, "Response Content = \n" + new String(mResponse));

		return true;
	}

	public byte[] getResponse()
	{
		return mResponse;
	}

	public void setResponse(byte[] response)
	{
		this.mResponse = response;
	}

	public Element ParseSoapResponse(String action) throws ParserConfigurationException, SAXException, IOException
	{
		String xml = new String(mResponse);
		xml = xml.substring(xml.indexOf('<'));
		InputSource inputSource = new InputSource(new ByteArrayInputStream(xml.getBytes()));
		DocumentBuilder documentBuilder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
		mDocument = documentBuilder.parse(inputSource);
		mElementEnvelope = (Element) mDocument.getElementsByTagName("s:Envelope").item(0);
		mElementBody = (Element) mElementEnvelope.getElementsByTagName("s:Body").item(0);

		return (Element) mElementBody.getElementsByTagName("u:" + action).item(0);
	}

	public HashMap<String, String> getResponseAttributes(Element elementAction)
	{
		HashMap<String, String> map = new HashMap<String, String>();

		NodeList nodeList = elementAction.getChildNodes();
		for (int i = 0; ; i++)
		{
			Node node = nodeList.item(i);
			if (node == null)
			{
				break;
			}

			if (node.getNodeType() != Node.ELEMENT_NODE)
			{
				continue;
			}

			Element element = (Element) node;

			map.put(element.getNodeName(), element.getTextContent());
		}

		return map;
	}
}
