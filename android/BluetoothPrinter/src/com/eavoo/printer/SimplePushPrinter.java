package com.eavoo.printer;

import java.io.IOException;
import android.content.Context;

public class SimplePushPrinter extends BppBase
{
    public SimplePushPrinter(Context context, BppObexTransport transport, String filename, String filetype)
	{
		super(context, transport, filename, filetype);
	}

	@Override
	public boolean BppObexRun()
	{
		try
		{
			if (PutFile(UUID_DPS, null))
			{
				return true;
			}
		}
		catch (IOException e)
		{
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		return false;
	}
}
