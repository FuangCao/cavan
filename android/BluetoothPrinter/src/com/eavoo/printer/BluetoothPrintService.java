package com.eavoo.printer;

import android.app.Service;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.os.Binder;
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

	public boolean SimplePushPrint(BluetoothDevice device, BluetoothPrintJob job)
	{
		CavanLog("Start SimplePushPrint");
		BppObexTransport transport = new BppObexTransport(device);
		SimplePushPrinter printer = new SimplePushPrinter(this, transport, job);
		printer.start();

		return true;
	}

	public boolean JobBasePrint(BluetoothDevice device, BluetoothPrintJob job)
	{
		BppObexTransport transport = new BppObexTransport(device);
		JobBasePrinter printer = new JobBasePrinter(this, transport, job);
		printer.start();

		return true;
	}
}
