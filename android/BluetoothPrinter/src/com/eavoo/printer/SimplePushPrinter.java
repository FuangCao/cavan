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
		boolean ret;

		try
		{
			ret = PutFile(getFileName(), getFileType(), null, UUID_DPS);
		}
		catch (IOException e)
		{
			e.printStackTrace();
			ret = false;
		}

		if (ret == false || WaitPrinterReady(5000) == false)
		{
			SendMessage(BPP_MSG_SIMPLE_PUSH_PRINT_COMPLETE, -1, null);
			return false;
		}

		SendMessage(BPP_MSG_SIMPLE_PUSH_PRINT_COMPLETE, 0, null);

		return true;
	}
}
