package com.eavoo.printer;

import java.io.IOException;

import javax.xml.parsers.ParserConfigurationException;

import org.xml.sax.SAXException;

import android.app.Service;
import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;

public class BluetoothPrintService extends Service
{
	// private static final String TAG = "BluetoothPrintService";
	private static final String PDF_TEMP_JPG = "pdf.jpg";

	BluetoothPrinterActivity mBluetoothPrinterActivity;

	void CavanLog(String msg)
	{
		Log.v("BluetoothPrintService", msg);
	}

	class BluetoothPrintBinder extends Binder
	{
		BluetoothPrintService getService()
		{
			return BluetoothPrintService.this;
		}
	}

	@Override
	public void onCreate()
	{
		Log.v("Cavan", "onCreate");
		super.onCreate();
	}

	@Override
	public void onDestroy()
	{
		Log.v("Cavan", "onDestroy");
		super.onDestroy();
	}

	@Override
	public IBinder onBind(Intent arg0)
	{
		Log.v("Cavan", "onBind");
		return new BluetoothPrintBinder();
	}

	public void setBluetoothPrinterActivity(BluetoothPrinterActivity activity)
	{
		this.mBluetoothPrinterActivity = activity;
	}

	class SimplePushPdfPrinter extends SimplePushPrinter
	{
		public SimplePushPdfPrinter(Context context, Handler handler, BppObexTransport transport, BluetoothPrintJob job)
		{
			super(context, handler, transport, job);
		}

		private boolean PrintPdfFile(String pdfpath, int start, int end) throws IOException
		{
			setProgressMessage(String.format("Print PDF file, start = %d, end = %d", start, end));

			while (start <= end)
			{
				String jpgpath;

				setProgressMessage(String.format("Convert PDF page %d to jpeg", start));

				try
				{
					jpgpath = mBluetoothPrinterActivity.Pdf2Jpeg(pdfpath, start, PDF_TEMP_JPG);
				}
				catch (IOException e1)
				{
					e1.printStackTrace();
					return false;
				}
				catch (InterruptedException e1)
				{
					e1.printStackTrace();
					return false;
				}

				if (jpgpath == null)
				{
					return false;
				}

				if (PutFile(jpgpath, BluetoothPrintJob.GetFileMimeTypeByName(jpgpath), null, UUID_DPS) == false)
				{
					return false;
				}

				if (WaitPrinterReady(5000) == false)
				{
					return false;
				}

				start++;
			}

			return true;
		}

		@Override
		public boolean BluetoothPrinterRun()
		{
			boolean ret = false;

			try
			{
				ret = PrintPdfFile(mPrintJob.getFileName(), mPrintJob.getPageStart(), mPrintJob.getPageEnd());
			}
			catch (IOException e)
			{
				e.printStackTrace();
			}

			if (ret)
			{
				SendMessage(BPP_MSG_SIMPLE_PUSH_PRINT_COMPLETE, 0);
			}
			else
			{
				SendMessage(BPP_MSG_SIMPLE_PUSH_PRINT_COMPLETE, -1);
			}

			return ret;
		}
	}

	public boolean SimplePushPrint()
	{
		Handler handler = mBluetoothPrinterActivity.getHandler();
		BluetoothDevice device = mBluetoothPrinterActivity.getBluetoothDevice();

		BluetoothPrintJob job = mBluetoothPrinterActivity.getPrintJob();

		BppObexTransport transport = new BppObexTransport(device);
		BluetoothBasePrinter printer;

		String extension = job.getFileExtension();
		if (extension.equals("pdf"))
		{
			printer = new SimplePushPdfPrinter(this, handler, transport, job);
		}
		else
		{
			printer = new SimplePushPrinter(this, handler, transport, job);
		}

		printer.start();

		return true;
	}

	class JobBasePdfPrinter extends JobBasePrinter
	{
		public JobBasePdfPrinter(Context context, Handler handler, BppObexTransport transport, BluetoothPrintJob job)
		{
			super(context, handler, transport, job);
		}

		private boolean PrintPdfFile(String pdfpath, int start, int end) throws IOException, ParserConfigurationException, SAXException
		{
			setProgressMessage(String.format("start = %d, end = %d", start, end));

			while (start <= end)
			{
				String jpgpath;

				setProgressTitle("Print PDF page " + start);
				setProgressMessage(String.format("Convert PDF page %d to jpeg", start));

				try
				{
					jpgpath = mBluetoothPrinterActivity.Pdf2Jpeg(pdfpath, start, PDF_TEMP_JPG);
				}
				catch (IOException e1)
				{
					e1.printStackTrace();
					return false;
				}
				catch (InterruptedException e1)
				{
					e1.printStackTrace();
					return false;
				}

				if (jpgpath == null)
				{
					return false;
				}

				mPrintJob.setDocumentFormat(BluetoothPrintJob.GetFileMimeTypeByName(jpgpath));

				if (PrintFile(jpgpath) == false)
				{
					return false;
				}

				start++;
			}

			return true;
		}

		@Override
		public boolean BluetoothPrinterRun()
		{
			boolean ret = false;

			try
			{
				ret = PrintPdfFile(mPrintJob.getFileName(), mPrintJob.getPageStart(), mPrintJob.getPageEnd());
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
				return true;
			}
			else
			{
				SendMessage(BPP_MSG_JOB_BASE_PRINT_COMPLETE, -1);
				return false;
			}
		}
	}

	public boolean JobBasePrint()
	{
		Handler handler = mBluetoothPrinterActivity.getHandler();
		BluetoothDevice device = mBluetoothPrinterActivity.getBluetoothDevice();

		BluetoothPrintJob job = mBluetoothPrinterActivity.getPrintJob();

		BppObexTransport transport = new BppObexTransport(device);
		BluetoothBasePrinter printer;

		String extension = job.getFileExtension();
		if (extension.equals("pdf"))
		{
			printer = new JobBasePdfPrinter(this, handler, transport, job);
		}
		else
		{
			printer = new JobBasePrinter(this, handler, transport, job);
		}

		printer.start();

		return true;
	}

	public boolean GetPrinterAttribute()
	{
		Handler handler = mBluetoothPrinterActivity.getHandler();
		BluetoothDevice device = mBluetoothPrinterActivity.getBluetoothDevice();
		BppObexTransport transport = new BppObexTransport(device);
		JobBasePrinter printer = new JobBasePrinter(this, handler, transport, null)
		{
			@Override
			public boolean BluetoothPrinterRun()
			{
				BluetoothPrinterAttribute attribute = GetPrinterAttributes();
				if (attribute == null)
				{
					SendMessage(BluetoothBasePrinter.BPP_MSG_GET_PRINTER_ATTRIBUTE_COMPLETE, -1);
				}
				else
				{
					Bundle data = new Bundle();
					data.putSerializable("PrinterAttribute", attribute);
					BuildMessage(BluetoothBasePrinter.BPP_MSG_GET_PRINTER_ATTRIBUTE_COMPLETE, 0, data).sendToTarget();
				}

				return true;
			}
		};

		printer.start();

		return false;
	}
}