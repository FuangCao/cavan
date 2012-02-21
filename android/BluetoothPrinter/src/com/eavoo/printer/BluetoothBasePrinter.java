package com.eavoo.printer;

import java.io.BufferedInputStream;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import javax.obex.ClientOperation;
import javax.obex.ClientSession;
import javax.obex.HeaderSet;
import javax.obex.ResponseCodes;

import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.Process;
import android.util.Log;

public class BluetoothBasePrinter extends Thread
{
	private static final String TAG = "BluetoothBasePrinter";
	private static boolean DEBUG = false;
	protected BppObexTransport mTransport;
	private WakeLock mWakeLock;
	protected BluetoothPrintJob mPrintJob;
	private Context mContext;
	private Handler mHandler;

	public static final int BPP_MSG_JOB_BASE_PRINT_COMPLETE = 0;
	public static final int BPP_MSG_SIMPLE_PUSH_PRINT_COMPLETE = 1;
	public static final int BPP_MSG_GET_PRINTER_ATTRIBUTE_COMPLETE = 2;
	public static final int BPP_MSG_GET_PRINTER_ATTRIBUTE_REQUEST = 3;
	public static final int BPP_MSG_CONNECT_FAILED = 4;

	public static final byte[] UUID_DPS =
	{
		0x00, 0x00, 0x11, 0x18, 0x00, 0x00, 0x10, 0x00,
		(byte) 0x80, 0x00, 0x00, (byte) 0x80, 0x5F, (byte) 0x9B, 0x34, (byte) 0xFB
	};

	public static final byte[] UUID_PBR =
	{
		0x00, 0x00, 0x11, 0x19, 0x00, 0x00, 0x10, 0x00,
		(byte) 0x80, 0x00, 0x00, (byte) 0x80, 0x5F, (byte) 0x9B, 0x34, (byte) 0xFB
	};

	public static final byte[] UUID_REF_OBJ =
	{
		0x00, 0x00, 0x11, 0x20, 0x00, 0x00, 0x10, 0x00,
		(byte) 0x80, 0x00, 0x00, (byte) 0x80, 0x5F, (byte) 0x9B, 0x34, (byte) 0xFB
	};

	public static final byte[] UUID_URI_REF_OBJ =
	{
		0x00, 0x00, 0x11, 0x21, 0x00, 0x00, 0x10, 0x00,
		(byte) 0x80, 0x00, 0x00, (byte) 0x80, 0x5F, (byte) 0x9B, 0x34, (byte) 0xFB
	};

	public static final byte[] UUID_STS =
	{
		0x00, 0x00, 0x11, 0x23, 0x00, 0x00, 0x10, 0x00,
		(byte) 0x80, 0x00, 0x00, (byte) 0x80, 0x5F, (byte) 0x9B, 0x34, (byte) 0xFB
	};

	public BluetoothBasePrinter(Context context, Handler handler, BppObexTransport transport, BluetoothPrintJob job)
	{
		Log.v(TAG, "Create BppBasePrinter");
		this.mContext = context;
		PowerManager pm = (PowerManager)mContext.getSystemService(Context.POWER_SERVICE);
		this.mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, TAG);

		this.mHandler = handler;
		this.mTransport = transport;
		this.mPrintJob = job;
	}

	public String getFileName()
	{
		return mPrintJob.getFileName();
	}

	public String getFileType()
	{
		return mPrintJob.getDocumentFormat();
	}

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

	public ClientSession connect(byte[] uuid) throws IOException
	{
		Log.v(TAG, "Create ClientSession with transport " + mTransport.toString());
		ClientSession session = new ClientSession(mTransport);
		HeaderSet hsRequest = new HeaderSet();

		if (uuid != null)
		{
			hsRequest.setHeader(HeaderSet.TARGET, uuid);
		}

		Log.v(TAG, "Connect to OBEX session");
		HeaderSet hsResponse = session.connect(hsRequest);
		Log.v(TAG, "ResponseCode = " + hsResponse.getResponseCode());

		byte[] headerWho = (byte[]) hsResponse.getHeader(HeaderSet.WHO);
		if (headerWho != null)
		{
			Log.v(TAG, "HeaderWho:\n" + ByteArrayToHexString(headerWho));
		}

		if (hsResponse.mConnectionID == null)
		{
			Log.v(TAG, "mConnectionID == null");
		}
		else
		{
			Log.v(TAG, "mConnectionID:\n" + ByteArrayToHexString(hsResponse.mConnectionID));
		}

		return hsResponse.getResponseCode() == ResponseCodes.OBEX_HTTP_OK ? session : null;
	}

	public boolean PutFile(InputStream inputStream, HeaderSet headerSet, byte[] uuid) throws IOException
	{
		ClientSession session = connect(uuid);
		if (session == null)
		{
			return false;
		}

		ClientOperation clientOperation = (ClientOperation) session.put(headerSet);
		if (clientOperation == null)
		{
			Log.e(TAG, "clientOperation == null");
			return false;
		}

		OutputStream obexOutputStream = clientOperation.openOutputStream();
		if (obexOutputStream == null)
		{
			Log.e(TAG, "obexOutputStream == null");
			clientOperation.abort();
			return false;
		}

		InputStream obexInputStream = clientOperation.openInputStream();
		if (obexInputStream == null)
		{
			Log.e(TAG, "obexInputStream == null");
			obexOutputStream.close();
			clientOperation.abort();
			return false;
		}

		BufferedInputStream bufferedInputStream = new BufferedInputStream(inputStream);
		int obexMaxPackageSize = clientOperation.getMaxPacketSize();
		byte[] buff = new byte[obexMaxPackageSize];

		boolean boolResult = true;

		Log.v(TAG, "Start send file");

		while (true)
		{
			int readLen = bufferedInputStream.read(buff);
			if (readLen <= 0)
			{
				Log.v(TAG, "Send file complete");
				break;
			}

			obexOutputStream.write(buff, 0, readLen);

			int responseCode = clientOperation.getResponseCode();
			if (responseCode != ResponseCodes.OBEX_HTTP_CONTINUE && responseCode != ResponseCodes.OBEX_HTTP_OK)
			{
				Log.e(TAG, "responseCode = " + responseCode);
				Log.e(TAG, "responseCode != ResponseCodes.OBEX_HTTP_CONTINUE && responseCode != ResponseCodes.OBEX_HTTP_OK");
				boolResult = false;
				break;
			}
		}

		bufferedInputStream.close();
		obexInputStream.close();
		obexOutputStream.close();

		if (boolResult)
		{
			clientOperation.close();
		}

		return boolResult;
	}

	public boolean PutFile(String filename, String filetype, HeaderSet headerSet, byte[] uuid) throws IOException
	{
		if (filename == null)
		{
			return false;
		}

		File file = new File(filename);
		long size = file.length();

		if (size <= 0)
		{
			Log.e(TAG, "File " + filename + " don't exist");
			return false;
		}

		if (headerSet == null)
		{
			headerSet = new HeaderSet();
		}

		headerSet.setHeader(HeaderSet.NAME, BluetoothPrintJob.FileBaseName(filename));
		Log.v(TAG, "NAME = " + headerSet.getHeader(HeaderSet.NAME));
		headerSet.setHeader(HeaderSet.LENGTH, size);
		Log.v(TAG, "LENGTH = " + headerSet.getHeader(HeaderSet.LENGTH));

		if (filetype == null)
		{
			filetype = BluetoothPrintJob.GetFileMimeTypeByName(filename);
		}
		headerSet.setHeader(HeaderSet.TYPE, filetype);
		Log.v(TAG, "TYPE = " + headerSet.getHeader(HeaderSet.TYPE));

		FileInputStream inputStream = new FileInputStream(file);
		boolean ret = PutFile(new FileInputStream(file), headerSet, uuid);
		inputStream.close();

		return ret;
	}

	public boolean PutByteArray(byte[] uuid, HeaderSet headerSet, byte[] data) throws IOException
	{
		if (headerSet == null)
		{
			headerSet = new HeaderSet();
		}

		headerSet.setHeader(HeaderSet.LENGTH, data.length);

		ByteArrayInputStream inputStream = new ByteArrayInputStream(data);

		return PutFile(inputStream, headerSet, uuid);
	}

	public byte[] GetByteArray(byte[] request, HeaderSet headerSet, byte[] uuid) throws IOException
	{
		if (DEBUG)
		{
			Log.v(TAG, "Request = \n" + new String(request));
		}

		ClientSession session = connect(uuid);
		if (session == null)
		{
			return null;
		}

		ClientOperation operation = (ClientOperation) session.get(headerSet);
		Log.v(TAG, "Get operation complete");

		OutputStream outputStream = operation.openOutputStream();
		Log.v(TAG, "Open OutputStream complete");
		outputStream.write(request);
		Log.v(TAG, "Write data complete");
		outputStream.close();

		int responseCode = operation.getResponseCode();
		if (responseCode != ResponseCodes.OBEX_HTTP_CONTINUE && responseCode != ResponseCodes.OBEX_HTTP_OK)
		{
			Log.e(TAG, "responseCode != ResponseCodes.OBEX_HTTP_CONTINUE && responseCode != ResponseCodes.OBEX_HTTP_OK");
			return null;
		}

		int length = (int) operation.getLength();
		Log.v(TAG, "length = " + length);
		if (length <= 0)
		{
			return null;
		}

		InputStream inputStream = operation.openInputStream();
		Log.v(TAG, "Open InputStream complete");
		byte[] response = new byte[length];
		inputStream.read(response);
		inputStream.close();

		if (DEBUG)
		{
			Log.v(TAG, "Response Content = \n" + new String(response));
		}

		return response;
	}

	public byte[] SendSoapRequest(byte[] request)
	{
		HeaderSet reqHeaderSet = new HeaderSet();
		reqHeaderSet.setHeader(HeaderSet.TYPE, "x-obex/bt-SOAP");

		try
		{
			byte[] response = GetByteArray(request, reqHeaderSet, null);

			return response;
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}

		return null;
	}

	public boolean BluetoothPrinterRun()
	{
		Log.e(TAG, "BluetoothPrinterRun No implementation");

		return false;
	}

	private Message BuildMessage(int what, int arg1, Bundle data)
	{
		Message message = Message.obtain(mHandler);
		message.what = what;
		message.arg1 = arg1;
		if (data != null)
		{
			message.setData(data);
		}

		return message;
	}

	public void SendMessage(int what, int arg1, Bundle data)
	{
		BuildMessage(what, arg1, data).sendToTarget();
	}

	public void SendMessageDelay(int what, int arg1, Bundle data, long delayMillis)
	{
		mHandler.sendMessageDelayed(BuildMessage(what, arg1, data), delayMillis);
	}

	@Override
	public void run()
	{
		Process.setThreadPriority(Process.THREAD_PRIORITY_BACKGROUND);

		Log.v(TAG, "Bpp Soap request running");

		mWakeLock.acquire();

		try
		{
			Log.v(TAG, "Connect to printer");
			mTransport.connect();
		}
		catch (IOException e1)
		{
			e1.printStackTrace();
			SendMessage(BPP_MSG_CONNECT_FAILED, -1, null);
			return;
		}

		if (BluetoothPrinterRun())
		{
			Log.v(TAG, "Print complete");
		}
		else
		{
			Log.v(TAG, "Print failed");
		}

		try
		{
			mTransport.close();
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}

		mWakeLock.release();
	}
}
