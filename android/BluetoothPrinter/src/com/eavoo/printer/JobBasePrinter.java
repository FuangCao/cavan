package com.eavoo.printer;

import java.io.IOException;

import javax.obex.HeaderSet;
import javax.xml.parsers.ParserConfigurationException;

import org.xml.sax.SAXException;

import android.content.Context;

import com.sun.pdfview.PDFFile;

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
		}

		return true;
	}

	public boolean PrintPdfFile(String filename) throws IOException, ParserConfigurationException, SAXException
	{
		final String filetype = "image/jpeg";
		PrintJob job = new PrintJob(this, filetype);
		PDFFile pdfFile = OpenPdfFile(filename);
		if (pdfFile == null)
		{
			return false;
		}

		int pageCount = pdfFile.getNumPages();
		if (pageCount <= 0)
		{
			return false;
		}

		CavanLog("PDF Page Count = " + pageCount);

		HeaderSet headerSet = new HeaderSet();
		headerSet.setHeader(HeaderSet.TYPE, filetype);

		boolean ret = true;
		for (int i = 0; i < pageCount; i++)
		{
			CavanLog("Start print page " + i);

			byte[] buff = PdfToJpeg(pdfFile, i);

			if (buff == null)
			{
				ret = false;
				break;
			}

			ret = job.CreateJob();
			if (ret == false)
			{
				break;
			}

			headerSet.setHeader(HeaderSet.APPLICATION_PARAMETER, job.buildApplicationParameter());

			ret = SendDocument(job, buff, headerSet);
			if (ret == false)
			{
				job.CancelJob();
				break;
			}

			CavanLog("Send page " + i + " complete");
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
			if (extension.equals("pdf"))
			{
				CavanLog("Print PDF File");
				ret = PrintPdfFile(getFileName());
			}
			else
			{
				CavanLog("Print Other File");
				ret = PrintFile(getFileName(), getFileType());
			}
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
