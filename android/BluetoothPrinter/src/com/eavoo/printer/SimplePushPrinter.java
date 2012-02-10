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
	// private String mFilePathName = "/mnt/sdcard/test.jpg";
	// private String mFilePathName = "/mnt/sdcard/BPP_SPEC_V12r00.pdf";
	private String mFilePathName = "/mnt/sdcard/init.txt";
	private String mFileMimeType = null;

    public SimplePushPrinter(Context context, BppObexTransport transport)
	{
		super(context, transport);
		// TODO Auto-generated constructor stub
	}

	public boolean PrintFile()
	{
		File file = new File(mFilePathName);
		long fileLength = file.length();

		if (fileLength == 0)
		{
			CavanLog("File " + mFilePathName + " don't exist");
			return false;
		}

		HeaderSet reqHeaderSet = new HeaderSet();

		int index = mFilePathName.lastIndexOf('/');
		if (index < 0)
		{
			reqHeaderSet.setHeader(HeaderSet.NAME, mFilePathName);
		}
		else
		{
			reqHeaderSet.setHeader(HeaderSet.NAME, mFilePathName.substring(index + 1));
		}

		if (mFileMimeType == null)
		{
			mFileMimeType = GetFileMimeTypeByName(mFilePathName);
		}

		reqHeaderSet.setHeader(HeaderSet.TYPE, mFileMimeType);
		reqHeaderSet.setHeader(HeaderSet.LENGTH, fileLength);

		try
		{
			CavanLog("NAME = " + reqHeaderSet.getHeader(HeaderSet.NAME));
			CavanLog("TYPE = " + reqHeaderSet.getHeader(HeaderSet.TYPE));
			CavanLog("LENGTH = " + reqHeaderSet.getHeader(HeaderSet.LENGTH));
		}
		catch (IOException e2)
		{
			// TODO Auto-generated catch block
			e2.printStackTrace();
		}

		ClientOperation clientOperation;
		try {
			clientOperation = (ClientOperation) mObexClientSession.put(reqHeaderSet);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			clientOperation = null;
		}

		if (clientOperation == null)
		{
			CavanLog("clientOperation == null");
			return false;
		}

		OutputStream obexOutputStream;
		try {
			obexOutputStream = clientOperation.openOutputStream();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			obexOutputStream = null;
		}

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

		InputStream obexInputStream;
		try {
			obexInputStream = clientOperation.openInputStream();
		} catch (IOException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
			obexInputStream = null;
		}

		if (obexInputStream == null)
		{
			CavanLog("obexInputStream == null");

			try {
				obexOutputStream.close();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}

			try {
				clientOperation.abort();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}

			return false;
		}

		FileInputStream fileInputStream;
		try {
			fileInputStream = new FileInputStream(file);
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			fileInputStream = null;
		}

		if (fileInputStream == null)
		{
			CavanLog("fileInputStream == null");

			try {
				obexInputStream.close();
			} catch (IOException e1) {
				// TODO Auto-generated catch block
				e1.printStackTrace();
			}

			try {
				obexOutputStream.close();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}

			try {
				clientOperation.abort();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			return false;
		}

		BufferedInputStream bufferedInputStream = new BufferedInputStream(fileInputStream);
		int obexMaxPackageSize = clientOperation.getMaxPacketSize();
		byte[] buff = new byte[obexMaxPackageSize];

		boolean boolResult = true;

		CavanLog("Start send file");

		while (fileLength != 0)
		{
			int sendLength;

			try {
				sendLength = bufferedInputStream.read(buff);
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
				sendLength = -1;
			}

			CavanLog("readLength = " + sendLength);

			if (sendLength < 0)
			{
				boolResult = false;
				break;
			}

			try {
				obexOutputStream.write(buff, 0, sendLength);
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
				boolResult = false;
				break;
			}

			int responseCode;
			try {
				responseCode = clientOperation.getResponseCode();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
				boolResult = false;
				break;
			}

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

		try {
			bufferedInputStream.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		try {
			obexInputStream.close();
		} catch (IOException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}

		try {
			fileInputStream.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		try {
			obexOutputStream.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		try {
			if (boolResult)
			{
				clientOperation.close();
			}
			else
			{
				clientOperation.abort();
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		return boolResult;
	}

	@Override
	public void run()
	{
		Process.setThreadPriority(Process.THREAD_PRIORITY_BACKGROUND);

		CavanLog("Printer thread running");

		mWakeLock.acquire();

		if (connect())
		{
			CavanLog("Connect successfully");
		}
		else
		{
			CavanLog("Connect failed");
			return;
		}

		if (PrintFile())
		{
			CavanLog("Print complete");
		}
		else
		{
			CavanLog("Print failed");
		}

		disconnect();

		mWakeLock.release();
	}
}
