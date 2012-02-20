package com.eavoo.printer;

import java.io.IOException;

import javax.obex.HeaderSet;
import javax.xml.parsers.ParserConfigurationException;

import org.xml.sax.SAXException;

import android.content.Context;
import android.os.Handler;

public class JobBasePrinter extends BluetoothBasePrinter
{
	public JobBasePrinter(Context context, Handler handler, BppObexTransport transport, BluetoothPrintJob job)
	{
		super(context, handler, transport, job);
	}

	public BluetoothPrinterAttribute GetPrinterAttributes()
	{
		BluetoothPrinterAttribute attribute = new BluetoothPrinterAttribute(this);

		try
		{
			attribute.update();
			return attribute;
		}
		catch (ParserConfigurationException e)
		{
			e.printStackTrace();
		}
		catch (SAXException e)
		{
			e.printStackTrace();
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}

		return null;
	}

	public boolean SendDocument() throws IOException
	{
		return PutFile(mPrintJob.getFileName(), null, mPrintJob.buildHeaderSet(), UUID_DPS);
	}

	public boolean SendDocument(BluetoothPrintJob job, byte[] data, HeaderSet headerSet) throws IOException
	{
		return PutByteArray(UUID_DPS, headerSet, data);
	}

	public boolean PrintFile() throws IOException, ParserConfigurationException, SAXException
	{
		if (mPrintJob.create(this) == false)
		{
			return false;
		}

		CavanLog("JobId = " + mPrintJob.getJobId());
		CavanLog(String.format("OperationStatus = 0x%04x", mPrintJob.getOperationStatus()));

		mPrintJob.getAttributes();

		if (SendDocument() == false)
		{
			mPrintJob.cancel();
			return false;
		}

		return true;
	}

	@Override
	public boolean BluetoothPrinterRun()
	{
		String extension = BluetoothPrintJob.GetFileExtension(getFileName());

		CavanLog("File extension = \"" + extension + "\"");

		boolean ret = false;

		try
		{
			ret = PrintFile();
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}
		catch (ParserConfigurationException e)
		{
			e.printStackTrace();
		}
		catch (SAXException e)
		{
			e.printStackTrace();
		}

		if (ret)
		{
			SendMessage(BPP_MSG_JOB_BASE_PRINT_COMPLETE, 0, null);
		}
		else
		{
			SendMessage(BPP_MSG_JOB_BASE_PRINT_COMPLETE, -1, null);
		}

		return ret;
	}
}
