package com.eavoo.printer;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.nio.charset.Charset;

import javax.obex.ClientOperation;
import javax.obex.ClientSession;
import javax.obex.HeaderSet;
import javax.obex.ResponseCodes;
import javax.xml.parsers.ParserConfigurationException;

import org.xml.sax.SAXException;

import android.content.Context;
import android.os.PowerManager;
import android.os.Process;
import android.os.PowerManager.WakeLock;
import android.util.Log;

public class JobBasePrinter extends BppBase
{
	public JobBasePrinter(Context context, BppObexTransport transport)
	{
		super(context, transport, null);
		// TODO Auto-generated constructor stub
	}

	private PrintJob CreateJob()
	{
		PrintJob printJob = new PrintJob(mObexClientSession);

		boolean ret = false;
		try
		{
			ret = printJob.CreateJob();
		}
		catch (IOException e)
		{
			// TODO Auto-generated catch block
			e.printStackTrace();
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

		if (ret == false)
		{
			return null;
		}

		CavanLog("JobId = " + printJob.getJobId());
		CavanLog("OperationStatus = " + printJob.getOperationStatus());

		return printJob;
	}

	@Override
	public boolean BppObexRun()
	{
		PrintJob job = CreateJob();

		job.CancelJob();

		return true;
	}
}
