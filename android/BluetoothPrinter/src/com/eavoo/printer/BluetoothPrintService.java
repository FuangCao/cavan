package com.eavoo.printer;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;

import javax.obex.HeaderSet;

import android.app.Service;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.os.Binder;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;

public class BluetoothPrintService extends Service
{
	private static final String TAG = "BluetoothPrintService";
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

	public boolean SimplePushPrint()
	{

		Handler handler = mBluetoothPrinterActivity.getHandler();
		BluetoothDevice device = mBluetoothPrinterActivity.getBluetoothDevice();
		BluetoothPrintJob job = mBluetoothPrinterActivity.getPrintJob();

		BppObexTransport transport = new BppObexTransport(device);
		SimplePushPrinter printer = new SimplePushPrinter(this, handler, transport, job);
		CavanLog("Start SimplePushPrint");
		printer.start();

		return true;
	}

	public boolean JobBasePrintOther()
	{
		Handler handler = mBluetoothPrinterActivity.getHandler();
		BluetoothDevice device = mBluetoothPrinterActivity.getBluetoothDevice();
		BluetoothPrintJob job = mBluetoothPrinterActivity.getPrintJob();

		BppObexTransport transport = new BppObexTransport(device);
		JobBasePrinter printer = new JobBasePrinter(this, handler, transport, job);
		printer.start();

		return true;
	}

	public boolean JobBasePrintPdf()
	{
		Handler handler = mBluetoothPrinterActivity.getHandler();
		BluetoothDevice device = mBluetoothPrinterActivity.getBluetoothDevice();
		BluetoothPrintJob job = mBluetoothPrinterActivity.getPrintJob();
		job.setDocumentFormat(BluetoothPrintJob.GetFileMimeTypeByExtension("jpg"));

		BppObexTransport transport = new BppObexTransport(device);
		JobBasePrinter printer = new JobBasePrinter(this, handler, transport, job)
		{
			@Override
			public boolean BluetoothPrinterRun()
			{
				int count;
				try
				{
					count = mBluetoothPrinterActivity.getPdfPageCount(mPrintJob.getFileName());
				}
				catch (IOException e)
				{
					count = -1;
					e.printStackTrace();
				}
				catch (InterruptedException e)
				{
					count = -1;
					e.printStackTrace();
				}

				if (count < 0)
				{
					Log.e(TAG, "getPdfPageCount failed");
					return false;
				}

				Log.v(TAG, "PdfPageCount = " + count);

				HeaderSet headerSet = mPrintJob.buildHeaderSet();
				headerSet.setHeader(HeaderSet.TYPE, mPrintJob.getDocumentFormat());

				boolean ret = true;

				for (int i = 0; i < count; i++)
				{
					File jpgfile;

					try
					{
						jpgfile = mBluetoothPrinterActivity.Pdf2Jpeg(mPrintJob.getFileName(), i, PDF_TEMP_JPG);
					}
					catch (IOException e1)
					{
						e1.printStackTrace();
						ret = false;
						break;
					}
					catch (InterruptedException e1)
					{
						e1.printStackTrace();
						ret = false;
						break;
					}

					InputStream inputStream;
					try
					{
						inputStream = new FileInputStream(jpgfile);
					}
					catch (FileNotFoundException e1)
					{
						e1.printStackTrace();
						ret = false;
						break;
					}

					Log.v(TAG, "Start print page = " + i);

					try
					{
						ret = PrintFile(inputStream);
					}
					catch (IOException e2)
					{
						e2.printStackTrace();
						ret = false;
					}

					try
					{
						inputStream.close();
					}
					catch (IOException e1)
					{
						e1.printStackTrace();
					}

					if (ret == false)
					{
						break;
					}

					ret = WaitPrinterReady(5000);
					if (ret == false)
					{
						break;
					}
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
		};
		printer.start();

		return true;
	}

	public boolean JobBasePrint()
	{
		BluetoothPrintJob job = mBluetoothPrinterActivity.getPrintJob();
		String extension = job.getFileExtension();
		if (extension.equals("pdf"))
		{
			return JobBasePrintPdf();
		}
		else
		{
			return JobBasePrintOther();
		}
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
					SendMessage(BluetoothBasePrinter.BPP_MSG_GET_PRINTER_ATTRIBUTE_COMPLETE, -1, null);
				}
				else
				{
					Bundle data = new Bundle();
					data.putSerializable("PrinterAttribute", attribute);
					SendMessage(BluetoothBasePrinter.BPP_MSG_GET_PRINTER_ATTRIBUTE_COMPLETE, 0, data);
				}

				return true;
			}
		};
		printer.start();

		return false;
	}
}
