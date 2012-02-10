package com.eavoo.printer;

import java.io.IOException;

import javax.obex.ClientSession;
import javax.obex.HeaderSet;
import javax.obex.ResponseCodes;

import android.content.Context;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.util.Log;
import android.webkit.MimeTypeMap;

public class BppBase extends Thread
{
	private static final String TAG = "BppBase";
	private BppObexTransport mTransport;
	protected ClientSession mObexClientSession;
	protected WakeLock mWakeLock;
	private Context mContext;

	public String ByteArrayToHexString(byte[] bs)
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

	public void CavanLog(String message)
	{
		Log.v("Cavan", "\033[1m" + message + "\033[0m");
	}

	public void CavanLog(byte[] bs)
	{
		CavanLog(ByteArrayToHexString(bs));
	}

	public BppBase(Context context, BppObexTransport transport)
	{
		CavanLog("Create PrintThread");
		PowerManager pm = (PowerManager)context.getSystemService(Context.POWER_SERVICE);
		this.mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, TAG);

		this.mContext = context;
		this.mTransport = transport;
	}

	public boolean connect()
	{
		CavanLog("Create ClientSession with transport " + mTransport.toString());

		try
		{
			CavanLog("Connect to printer");
			mTransport.connect();
			CavanLog("Create OBEX client session");
			mObexClientSession = new ClientSession(mTransport);
		}
		catch (IOException e1)
		{
			CavanLog("OBEX session create error");
			return false;
		}

		HeaderSet hsRequest = new HeaderSet();

		hsRequest.setHeader(HeaderSet.TARGET, BppObexTransport.UUID_DPS);

		try
		{
			Log.d(TAG, "Connect to OBEX session");
			HeaderSet hsResponse = mObexClientSession.connect(hsRequest);
			CavanLog("ResponseCode = " + hsResponse.getResponseCode());

			byte[] headerWho = (byte[]) hsResponse.getHeader(HeaderSet.WHO);
			if (headerWho != null)
			{
				CavanLog("HeaderWho:");
				CavanLog(headerWho);
			}

			if (hsResponse.mConnectionID == null)
			{
				CavanLog("mConnectionID == null");
			}
			else
			{
				CavanLog(hsResponse.mConnectionID);
			}

			if (hsResponse.getResponseCode() == ResponseCodes.OBEX_HTTP_OK)
			{
				return true;
			}
		} catch (IOException e) {
			CavanLog("OBEX session connect error");
		}

		try {
			mTransport.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		return false;
	}

	public void disconnect()
	{
		CavanLog("disconnect");

		if (mObexClientSession != null)
		{
			try {
				mObexClientSession.disconnect(null);
				mObexClientSession.close();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}

		if (mTransport != null)
		{
			try {
				mTransport.close();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}

	public String GetFileMimeTypeByName(String pathname)
	{
            String extension;

            int dotIndex = pathname.lastIndexOf(".");
            if (dotIndex < 0)
            {
            	extension = "txt";
            }
            else
            {
            	extension = pathname.substring(dotIndex + 1).toLowerCase();
            }

            MimeTypeMap map = MimeTypeMap.getSingleton();

            return map.getMimeTypeFromExtension(extension);
	}
}
