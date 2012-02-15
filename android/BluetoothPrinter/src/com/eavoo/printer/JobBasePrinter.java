package com.eavoo.printer;

import java.io.IOException;
import javax.obex.ApplicationParameter;
import javax.obex.HeaderSet;
import javax.xml.parsers.ParserConfigurationException;
import org.xml.sax.SAXException;
import android.content.Context;

public class JobBasePrinter extends BppBase
{
	// private static final byte AppTagOffset = 1;
	// private static final byte AppTagCount = 2;
	private static final byte AppTagJobId = 3;
	// private static final byte AppTagFileSize = 4;

	public JobBasePrinter(Context context, BppObexTransport transport, String filename, String filetype)
	{
		super(context, transport, filename, filetype);
	}

	public static byte[] IntegerToByteArray(int value)
	{
		byte[] bs =
		{
			(byte) ((value >> 24) & 0xFF),
			(byte) ((value >> 16) & 0xFF),
			(byte) ((value >> 8) & 0xFF),
			(byte) (value & 0xFF),
		};

		return bs;
	}

	private PrintJob CreateJob()
	{
		PrintJob printJob = new PrintJob(this);

		printJob.setDocumentFormat(getFileType());

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

		return printJob;
	}

	public void GetPrinterAttributes()
	{
		BppSoapRequest request = new BppSoapRequest(this);

		request.setAttributes("GetPrinterAttributes", null, null);
		request.SendTo();
	}

	public boolean SendDocument(PrintJob job)
	{
		ApplicationParameter parameter = new ApplicationParameter();
		parameter.addAPPHeader(AppTagJobId, (byte) 4, IntegerToByteArray(job.getJobId()));

		HeaderSet headerSet = new HeaderSet();
		headerSet.setHeader(HeaderSet.APPLICATION_PARAMETER, parameter.getAPPparam());

		try
		{
			return PutFile(UUID_DPS, headerSet);
		}
		catch (IOException e)
		{
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		return false;
	}

	@Override
	public boolean BppObexRun()
	{
		PrintJob job = CreateJob();
		if (job == null)
		{
			return false;
		}

		CavanLog("JobId = " + job.getJobId());
		CavanLog(String.format("OperationStatus = 0x%04x", job.getOperationStatus()));

		job.GetJobAttributes();

		if (SendDocument(job) == false)
		{
			job.CancelJob();
		}

		// GetPrinterAttributes();

		return true;
	}
}
