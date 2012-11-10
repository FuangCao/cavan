package com.eavoo.printer;

import java.io.ByteArrayInputStream;
import java.io.IOException;
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
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

public class BppSoapRequest
{
	protected BluetoothBasePrinter mPrinter;
	private String mAction;
	private String mBody;
	private String mHttpHeader;
	private byte[] mResponse;
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

	public BppSoapRequest(BluetoothBasePrinter printer)
	{
		this.mPrinter = printer;
	}

	public byte[] getResponse()
	{
		return mResponse;
	}

	public void setResponse(byte[] response)
	{
		this.mResponse = response;
	}

	public void setAttributes(String action, String body, String header)
	{
		this.mAction = action;
		this.mBody = body;
		this.mHttpHeader = header;
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
		builderHeader.append(String.format("CONTENT-LENGTH: %8d\r\n", builderBody.length()));
		if (mHttpHeader != null)
		{
			builderHeader.append(mHttpHeader + "\r\n");
		}
		builderHeader.append("CONTENT-TYPE: text/xml; charset=\"utf-8\"\r\n");
		builderHeader.append("CONTENT-LANGUAGE: en-US\r\n");
		builderHeader.append("SOAPACTION: \"urn:schemas-bluetooth-org:service:Printer:1#" + mAction + "\"");

		String soapContent = builderHeader.toString() + "\r\n\r\n" + builderBody.toString();

		return soapContent.getBytes();
	}

	public boolean SendTo()
	{
		mResponse = mPrinter.SendSoapRequest(toByteArray());
		if (mResponse == null)
		{
			return false;
		}

		return true;
	}

	public boolean SendRequest(String action, String body, String header)
	{
		setAttributes(action, body, header);
		return SendTo();
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

	public HashMap<String, String> getResponseAttributes(String action)
	{
		Element elementAction = null;

		try
		{
			elementAction = ParseSoapResponse(action);
		}
		catch (ParserConfigurationException e)
		{
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		catch (SAXException e)
		{
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		catch (IOException e)
		{
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		if (elementAction == null)
		{
			return null;
		}

		return getResponseAttributes(elementAction);
	}

	public String getElementContent(Element parent, String name, String defvalue)
	{
		Element element  = (Element) parent.getElementsByTagName(name).item(0);

		return element == null ? defvalue : element.getTextContent();
	}

	public int getElementContentInt(Element parent, String name, int defvalue)
	{
		String string = getElementContent(parent, name, null);
		if (string == null)
		{
			return defvalue;
		}

		return Integer.decode(string);
	}

	public boolean getElementContentBoolean(Element parent, String name, boolean defvalue)
	{
		String string = getElementContent(parent, name, null);
		if (string == null)
		{
			return defvalue;
		}

		return Boolean.parseBoolean(string.toLowerCase());
	}

	public List<String> getElementContents(Element parent, String tag, String attr, List<String> defvalue)
	{
		Element element = (Element) parent.getElementsByTagName(tag).item(0);
		if (element == null)
		{
			return defvalue;
		}

		NodeList nodes = element.getElementsByTagName(attr);
		if (nodes == null)
		{
			return defvalue;
		}

		ArrayList<String> list = new ArrayList<String>();

		for (int i = 0; ; i++)
		{
			Node node = nodes.item(i);
			if (node == null)
			{
				break;
			}

			if (node.getNodeType() != Node.ELEMENT_NODE)
			{
				continue;
			}

			element = (Element) node;
			list.add(element.getTextContent());
		}

		return list;
	}
}
