package com.eavoo.printer;

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

	public boolean SimplePushPrint(Handler handler, BluetoothDevice device, BluetoothPrintJob job)
	{
		CavanLog("Start SimplePushPrint");
		BppObexTransport transport = new BppObexTransport(device);
		SimplePushPrinter printer = new SimplePushPrinter(this, handler, transport, job);
		printer.start();

		return true;
	}

	public boolean JobBasePrint(Handler handler, BluetoothDevice device, BluetoothPrintJob job)
	{
		BppObexTransport transport = new BppObexTransport(device);
		JobBasePrinter printer = new JobBasePrinter(this, handler, transport, job);
		printer.start();

		return true;
	}

	public boolean GetPrinterAttribute(Handler handler, BluetoothDevice device)
	{
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
