package com.eavoo.printer;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import javax.obex.ClientOperation;
import javax.obex.ClientSession;
import javax.obex.HeaderSet;
import javax.obex.ResponseCodes;

import android.content.Context;
import android.os.PowerManager;
import android.os.Process;
import android.os.PowerManager.WakeLock;
import android.util.Log;
import android.webkit.MimeTypeMap;

public class SimplePushPrinter extends BppBase
{
	private static final String TAG = "SimplePushPrinter";

    public SimplePushPrinter(Context context, BppObexTransport transport)
	{
		super(context, transport, BppObexTransport.UUID_DPS);
		// TODO Auto-generated constructor stub
	}

	public boolean PrintFile() throws IOException
	{
		File file = new File(mFilePath);
		long fileLength = file.length();

		if (fileLength == 0)
		{
			CavanLog("File " + mFilePath + " don't exist");
			return false;
		}

		HeaderSet reqHeaderSet = new HeaderSet();

		int index = mFilePath.lastIndexOf('/');
		if (index < 0)
		{
			reqHeaderSet.setHeader(HeaderSet.NAME, mFilePath);
		}
		else
		{
			reqHeaderSet.setHeader(HeaderSet.NAME, mFilePath.substring(index + 1));
		}

		if (mFileType == null)
		{
			mFileType = GetFileMimeTypeByName(mFileType);
		}

		reqHeaderSet.setHeader(HeaderSet.TYPE, mFileType);
		reqHeaderSet.setHeader(HeaderSet.LENGTH, fileLength);

		CavanLog("NAME = " + reqHeaderSet.getHeader(HeaderSet.NAME));
		CavanLog("TYPE = " + reqHeaderSet.getHeader(HeaderSet.TYPE));
		CavanLog("LENGTH = " + reqHeaderSet.getHeader(HeaderSet.LENGTH));

		ClientOperation clientOperation = (ClientOperation) mObexClientSession.put(reqHeaderSet);
		if (clientOperation == null)
		{
			CavanLog("clientOperation == null");
			return false;
		}

		OutputStream obexOutputStream = clientOperation.openOutputStream();
		if (obexOutputStream == null)
		{
			CavanLog("obexOutputStream == null");
			try {
				clientOperation.abort();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			return false;
		}

		InputStream obexInputStream = clientOperation.openInputStream();
		if (obexInputStream == null)
		{
			CavanLog("obexInputStream == null");
			obexOutputStream.close();
			clientOperation.abort();

			return false;
		}

		FileInputStream fileInputStream = new FileInputStream(file);
		BufferedInputStream bufferedInputStream = new BufferedInputStream(fileInputStream);
		int obexMaxPackageSize = clientOperation.getMaxPacketSize();
		byte[] buff = new byte[obexMaxPackageSize];

		boolean boolResult = true;

		CavanLog("Start send file");

		while (fileLength != 0)
		{
			int sendLength = bufferedInputStream.read(buff);

			CavanLog("readLength = " + sendLength);

			if (sendLength < 0)
			{
				boolResult = false;
				break;
			}

			obexOutputStream.write(buff, 0, sendLength);

			int responseCode = clientOperation.getResponseCode();

			CavanLog("responseCode = " + responseCode);

			if (responseCode != ResponseCodes.OBEX_HTTP_CONTINUE && responseCode != ResponseCodes.OBEX_HTTP_OK)
			{
				CavanLog("responseCode != ResponseCodes.OBEX_HTTP_CONTINUE && responseCode != ResponseCodes.OBEX_HTTP_OK");
				boolResult = false;
				break;
			}

			fileLength -= sendLength;

			CavanLog("sendLength = " + sendLength + ", fileLength = " + fileLength);
		}

		bufferedInputStream.close();
		obexInputStream.close();
		fileInputStream.close();
		obexOutputStream.close();

		if (boolResult)
		{
			clientOperation.close();
		}
		else
		{
				clientOperation.abort();
		}

		return boolResult;
	}

	@Override
	public boolean BppObexRun()
	{
		try
		{
			if (PrintFile())
			{
				CavanLog("Print complete");
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
