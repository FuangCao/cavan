package com.eavoo.printer;

import java.io.IOException;
import javax.obex.HeaderSet;
import javax.xml.parsers.ParserConfigurationException;
import org.xml.sax.SAXException;
import android.content.Context;

public class JobBasePrinter extends BppBase
{
	public JobBasePrinter(Context context, BppObexTransport transport, String filename, String filetype)
	{
		super(context, transport, filename, filetype);
	}

	public void GetPrinterAttributes()
	{
		BppSoapRequest request = new BppSoapRequest(this);

		request.setAttributes("GetPrinterAttributes", null, null);
		request.SendTo();
	}

	public boolean SendDocument(PrintJob job, String filename) throws IOException
	{
		return PutFile(filename, null, job.buildHeaderSet(), UUID_DPS);
	}

	public boolean SendDocument(PrintJob job, byte[] data, HeaderSet headerSet) throws IOException
	{
		return PutByteArray(UUID_DPS, headerSet, data);
	}

	public boolean PrintFile(String filename, String filetype) throws IOException, ParserConfigurationException, SAXException
	{
		if (filetype == null)
		{
			filetype = GetFileMimeTypeByName(filename);
		}

		PrintJob job = new PrintJob(this, filetype);
		if (job.CreateJob() == false)
		{
			return false;
		}

		CavanLog("JobId = " + job.getJobId());
		CavanLog(String.format("OperationStatus = 0x%04x", job.getOperationStatus()));

		job.GetJobAttributes();

		if (SendDocument(job, filename) == false)
		{
			job.CancelJob();
			return false;
		}

		return true;
	}

	@Override
	public boolean BppObexRun()
	{
		String extension = GetFileExtension(getFileName());

		CavanLog("File extension = \"" + extension + "\"");

		boolean ret = false;

		try
		{
			ret = PrintFile(getFileName(), getFileType());
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

		GetPrinterAttributes();

		return ret;
	}
}
