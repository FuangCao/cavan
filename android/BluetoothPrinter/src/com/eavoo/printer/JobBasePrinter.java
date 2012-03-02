package com.eavoo.printer;

import java.io.IOException;
import java.io.InputStream;

import javax.obex.HeaderSet;
import javax.xml.parsers.ParserConfigurationException;

import org.xml.sax.SAXException;

import android.content.Context;
import android.os.Handler;

public class JobBasePrinter extends BluetoothBasePrinter
{
	@SuppressWarnings("unused")
	private static final String TAG = "JobBasePrinter";

	public JobBasePrinter(Context context, Handler handler, BppObexTransport transport, BluetoothPrintJob job)
	{
		super(context, handler, transport, job);
	}

	public BluetoothPrinterAttribute GetPrinterAttributes()
	{
		BluetoothPrinterAttribute attribute = new BluetoothPrinterAttribute(this);

		return attribute.update() ? attribute : null;
	}

	public boolean SendDocument(String filename) throws IOException
	{
		return PutFile(filename, mPrintJob.getDocumentFormat(), mPrintJob.buildHeaderSet(), UUID_DPS);
	}

	public boolean SendDocument(InputStream inputStream) throws IOException
	{
		HeaderSet headerSet = mPrintJob.buildHeaderSet();
		headerSet.setHeader(HeaderSet.TYPE, getFileType());

		return PutFile(inputStream, headerSet, UUID_DPS);
	}

	public boolean SendDocument(byte[] data, HeaderSet headerSet) throws IOException
	{
		return PutByteArray(UUID_DPS, headerSet, data);
	}

	public boolean WaitJobComplete(long interval)
	{
		while (true)
		{
			try
			{
				sleep(interval);
			}
			catch (InterruptedException e)
			{
				e.printStackTrace();
			}

			if (mPrintJob.getAttributes() == false)
			{
				return false;
			}

			if (mPrintJob.getJobState().equals("completed"))
			{
				break;
			}
		}

		return true;
	}

	public boolean PrintFile(String filename) throws IOException, ParserConfigurationException, SAXException
	{
		setProgressMessage("Create print job");

		if (mPrintJob.create(this) == false)
		{
			return false;
		}

		mPrintJob.getAttributes();

		if (SendDocument(filename) == false)
		{
			mPrintJob.cancel();
			return false;
		}

		return WaitPrinterReady(5000);
	}

	public boolean PrintFile(InputStream inputStream) throws IOException
	{
		if (mPrintJob.create(this) == false)
		{
			return false;
		}

		mPrintJob.getAttributes();

		if (SendDocument(inputStream) == false)
		{
			mPrintJob.cancel();
			return false;
		}

		return true;
	}

	@Override
	public boolean BluetoothPrinterRun()
	{
		boolean ret = false;

		try
		{
			ret = PrintFile(mPrintJob.getFileName());
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
			SendMessage(BPP_MSG_JOB_BASE_PRINT_COMPLETE, 0);
		}
		else
		{
			SendMessage(BPP_MSG_JOB_BASE_PRINT_COMPLETE, -1);
		}

		return ret;
	}
}
