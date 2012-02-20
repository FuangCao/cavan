package com.eavoo.printer;

import java.io.IOException;

import android.content.Context;

public class SimplePushPrinter extends BluetoothBasePrinter
{
    public SimplePushPrinter(Context context, BppObexTransport transport, BluetoothPrintJob job)
	{
		super(context, transport, job);
	}

	@Override
	public boolean BluetoothPrinterRun()
	{
		try
		{
			if (PutFile(getFileName(), getFileType(), null, UUID_DPS))
			{
				return true;
			}
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}

		return false;
	}
}
