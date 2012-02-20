package com.eavoo.printer;

import java.io.IOException;

import android.content.Context;
import android.os.Handler;

public class SimplePushPrinter extends BluetoothBasePrinter
{
    public SimplePushPrinter(Context context, Handler handler, BppObexTransport transport, BluetoothPrintJob job)
	{
		super(context, handler, transport, job);
	}

	@Override
	public boolean BluetoothPrinterRun()
	{
		try
		{
			if (PutFile(getFileName(), getFileType(), null, UUID_DPS))
			{
				SendMessage(BPP_MSG_SIMPLE_PUSH_PRINT_COMPLETE, 0, null);
				return true;
			}
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}

		SendMessage(BPP_MSG_SIMPLE_PUSH_PRINT_COMPLETE, -1, null);

		return false;
	}
}
