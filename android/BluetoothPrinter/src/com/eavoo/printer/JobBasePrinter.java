package com.eavoo.printer;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import javax.obex.ClientOperation;
import javax.obex.ClientSession;
import javax.obex.HeaderSet;
import javax.obex.ResponseCodes;

import android.content.Context;
import android.os.PowerManager;
import android.os.Process;
import android.os.PowerManager.WakeLock;
import android.util.Log;

public class JobBasePrinter extends BppBase
{
	public JobBasePrinter(Context context, BppObexTransport transport)
	{
		super(context, transport);
		// TODO Auto-generated constructor stub
	}

	public boolean SendByteArray(byte[] buff)
	{
		HeaderSet reqHeaderSet = new HeaderSet();

		reqHeaderSet.setHeader(HeaderSet.TYPE, "x-obex/bt-SOAP");
		ClientOperation clientOperation;

		try
		{
			clientOperation = (ClientOperation) mObexClientSession.get(reqHeaderSet);
		}
		catch (IOException e)
		{
			// TODO Auto-generated catch block
			e.printStackTrace();
			return false;
		}

		CavanLog("Get operation complete");

		OutputStream outputStream;
		try
		{
			outputStream = clientOperation.openOutputStream();
		}
		catch (IOException e)
		{
			// TODO Auto-generated catch block
			e.printStackTrace();
			return false;
		}

		CavanLog("Open OutputStream complete");

		InputStream inputStream;
		try
		{
			inputStream = clientOperation.openInputStream();
		}
		catch (IOException e)
		{
			// TODO Auto-generated catch block
			e.printStackTrace();
			return false;
		}

		CavanLog("Open InputStream complete");

		try
		{
			outputStream.write(buff);
		}
		catch (IOException e)
		{
			// TODO Auto-generated catch block
			e.printStackTrace();
			return false;
		}

		int responseCode;
		try
		{
			responseCode = clientOperation.getResponseCode();
		}
		catch (IOException e)
		{
			// TODO Auto-generated catch block
			e.printStackTrace();
			return false;
		}

		if (responseCode != ResponseCodes.OBEX_HTTP_CONTINUE && responseCode != ResponseCodes.OBEX_HTTP_OK)
		{
			CavanLog("responseCode != ResponseCodes.OBEX_HTTP_CONTINUE && responseCode != ResponseCodes.OBEX_HTTP_OK");
			return false;
		}

		return true;
	}

	public boolean SendRequest(String request, String body)
	{
		StringBuilder builderBody = new StringBuilder();

		builderBody.append("CONTENT-TYPE: text/xml; charset=\"utf-8\"\n");
		builderBody.append("SOAPACTION: \"urn:schemas-bluetooth-org:service:Printer:1#GetPrinterAttributes\"\n");
		builderBody.append("<s:Envelope\n");
		builderBody.append("\txmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"\n");
		builderBody.append("\ts:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\n");
		builderBody.append("\t<s:Body>\n");
		builderBody.append("\t\t<u:GetPrinterAttributes xmlns:u=\"urn:schemas-bluetooth-org:service:Printer:1\">\n");
		builderBody.append("\t\t\t<RequestedPrinterAttributes>\n");
		builderBody.append("\t\t\t\t<PrinterAttribute>PrinterName </PrinterAttribute>\n");
		builderBody.append("\t\t\t\t<PrinterAttribute>PrinterState </PrinterAttribute>\n");
		builderBody.append("\t\t\t\t<PrinterAttribute>PrinterStateReasons </PrinterAttribute>\n");
		builderBody.append("\t\t\t</RequestedPrinterAttributes>\n");
		builderBody.append("\t\t</u:GetPrinterAttributes>\n");
		builderBody.append("\t</s:Body>\n");
		builderBody.append("</s:Envelope>\n");

		body = "CONTENT-LENGTH: " + builderBody.length() + "\n" + builderBody.toString();

		CavanLog("Body = " + body);

		return SendByteArray(builderBody.toString().getBytes());
	}

	@Override
	public void run()
	{
		Process.setThreadPriority(Process.THREAD_PRIORITY_BACKGROUND);

		CavanLog("Printer thread running");

		mWakeLock.acquire();

		if (connect())
		{
			CavanLog("Connect successfully");
		}
		else
		{
			CavanLog("Connect failed");
			return;
		}

		if (SendRequest(null, null))
		{
			CavanLog("Print complete");
		}
		else
		{
			CavanLog("Print failed");
		}

		disconnect();

		mWakeLock.release();
	}
}
