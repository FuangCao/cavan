package com.eavoo.printer;

import android.util.Log;

public class JobBasePrinter
{
	private String ByteArrayToHexString(byte[] bs)
	{
		if (bs == null)
		{
			return "";
		}

		StringBuilder stringBuilder = new StringBuilder();

		for (byte b : bs)
		{
			if ((b & 0xF0) == 0)
			{
				stringBuilder.append("0");
			}

			stringBuilder.append(Integer.toHexString((b >> 4) & 0x0F) + Integer.toHexString(b & 0x0F));
		}

		return stringBuilder.toString();
	}

	private void CavanLog(String message)
	{
		Log.v("Cavan", "\033[1m" + message + "\033[0m");
	}

	private void CavanLog(byte[] bs)
	{
		CavanLog(ByteArrayToHexString(bs));
	}

	public JobBasePrinter()
	{
		// TODO Auto-generated constructor stub
	}

	private boolean SendRequest(String request)
	{
		return false;
	}
}
