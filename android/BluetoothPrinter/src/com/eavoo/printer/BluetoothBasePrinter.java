package com.eavoo.printer;

import java.io.BufferedInputStream;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;

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
	public static final int BPP_MSG_SET_PROGRESS_MESSAGE = 5;
	public static final int BPP_MSG_SET_PROGRESS_TITLE = 6;

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

	public static String ByteToString(byte b)
	{
		StringBuilder builder = new StringBuilder();

		for (int i = 4; i >= 0; i -= 4)
		{
			int temp = (b >> i) & 0x0F;

			if (temp < 10)
			{
				builder.append((char) ('0' + temp));
			}
			else
			{
				builder.append((char) ('A' - 10 + temp));
			}
		}

		return builder.toString();
	}

	public static String ByteArrayToString(byte[] bs, int start, int length)
	{
		StringBuilder builder = new StringBuilder();

		for (int end = start + length; start < end; start++)
		{
			builder.append(ByteToString(bs[start]));
		}

		return builder.toString();
	}

	public static String ByteArrayToString(byte[] bs)
	{
		StringBuilder builder = new StringBuilder();

		builder.append(ByteArrayToString(bs, 0, 4) + "-");
		builder.append(ByteArrayToString(bs, 4, 2) + "-");
		builder.append(ByteArrayToString(bs, 6, 2) + "-");
		builder.append(ByteArrayToString(bs, 8, 2) + "-");
		builder.append(ByteArrayToString(bs, 10, 6));

		return builder.toString();
	}

	public static UUID ByteArrayToUUID(byte[] bs)
	{
		return UUID.fromString(ByteArrayToString(bs));
	}

	public BluetoothBasePrinter(Context context, Handler handler, BppObexTransport transport, BluetoothPrintJob job)
	{
		DebugLog("Create BppBasePrinter");
		this.mContext = context;
		PowerManager pm = (PowerManager)mContext.getSystemService(Context.POWER_SERVICE);
		this.mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, TAG);

		this.mHandler = handler;
		this.mTransport = transport;
		this.mPrintJob = job;
	}

	public void CavanLog(String message)
	{
		Log.v(TAG, message);
	}

	public void DebugLog(String message)
	{
		if (DEBUG)
		{
			Log.d(TAG, message);
		}
	}

	public void ErrorLog(String message)
	{
		Log.e(TAG, message);
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
		DebugLog("Create ClientSession with transport " + mTransport.toString());
		ClientSession session = new ClientSession(mTransport);
		HeaderSet hsRequest = new HeaderSet();

		if (uuid != null)
		{
			hsRequest.setHeader(HeaderSet.TARGET, uuid);
		}

		DebugLog("Connect to OBEX session");
		HeaderSet hsResponse = session.connect(hsRequest);
		DebugLog("ResponseCode = " + hsResponse.getResponseCode());

		byte[] headerWho = (byte[]) hsResponse.getHeader(HeaderSet.WHO);
		if (headerWho != null)
		{
			DebugLog("HeaderWho:\n" + ByteArrayToHexString(headerWho));
		}

		if (hsResponse.mConnectionID == null)
		{
			DebugLog("mConnectionID == null");
		}
		else
		{
			DebugLog("mConnectionID:\n" + ByteArrayToHexString(hsResponse.mConnectionID));
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
			ErrorLog("clientOperation == null");
			return false;
		}

		OutputStream obexOutputStream = clientOperation.openOutputStream();
		if (obexOutputStream == null)
		{
			ErrorLog("obexOutputStream == null");
			clientOperation.abort();
			return false;
		}

		InputStream obexInputStream = clientOperation.openInputStream();
		if (obexInputStream == null)
		{
			ErrorLog("obexInputStream == null");
			obexOutputStream.close();
			clientOperation.abort();
			return false;
		}

		BufferedInputStream bufferedInputStream = new BufferedInputStream(inputStream);
		int obexMaxPackageSize = clientOperation.getMaxPacketSize();
		byte[] buff = new byte[obexMaxPackageSize];

		boolean boolResult = true;

		int filesize = inputStream.available();

		while (true)
		{
			int readLen = bufferedInputStream.read(buff);
			if (readLen <= 0)
			{
				setProgressMessage("Send file complete");
				break;
			}

			obexOutputStream.write(buff, 0, readLen);

			int responseCode = clientOperation.getResponseCode();
			if (responseCode != ResponseCodes.OBEX_HTTP_CONTINUE && responseCode != ResponseCodes.OBEX_HTTP_OK)
			{
				ErrorLog("responseCode = " + responseCode);
				ErrorLog("responseCode != ResponseCodes.OBEX_HTTP_CONTINUE && responseCode != ResponseCodes.OBEX_HTTP_OK");
				boolResult = false;
				break;
			}

			filesize -= readLen;
			setProgressMessage(String.format("Send file to printer\nremain = %dkB", filesize >> 10));
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
			ErrorLog("File " + filename + " don't exist");
			return false;
		}

		if (headerSet == null)
		{
			headerSet = new HeaderSet();
		}

		headerSet.setHeader(HeaderSet.NAME, BluetoothPrintJob.FileBaseName(filename));
		DebugLog("NAME = " + headerSet.getHeader(HeaderSet.NAME));
		headerSet.setHeader(HeaderSet.LENGTH, size);
		DebugLog("LENGTH = " + headerSet.getHeader(HeaderSet.LENGTH));

		if (filetype == null)
		{
			filetype = BluetoothPrintJob.GetFileMimeTypeByName(filename);
		}
		headerSet.setHeader(HeaderSet.TYPE, filetype);
		DebugLog("TYPE = " + headerSet.getHeader(HeaderSet.TYPE));

		FileInputStream inputStream = new FileInputStream(file);
		boolean ret = PutFile(new FileInputStream(file), headerSet, uuid);
		inputStream.close();

		return ret;
	}

	public boolean PutCommanOutput(String command, HeaderSet headerSet, byte[] uuid) throws InterruptedException, IOException
	{
		DebugLog("command = " + command);
		Process process = Runtime.getRuntime().exec(command);

		InputStream inputStream = process.getInputStream();
		DebugLog("inputStream.available() = " + inputStream.available());
		boolean ret = PutFile(inputStream, headerSet, uuid);
		inputStream.close();

		if (ret == false)
		{
			process.destroy();
			return false;
		}

		process.waitFor();
		if (process.exitValue() != 0)
		{
			ErrorLog("process.exitValue() = " + process.exitValue());
			return false;
		}

		return true;
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
		DebugLog("Request = \n" + new String(request));

		ClientSession session = connect(uuid);
		if (session == null)
		{
			return null;
		}

		ClientOperation operation = (ClientOperation) session.get(headerSet);
		DebugLog("Get operation complete");

		OutputStream outputStream = operation.openOutputStream();
		DebugLog("Open OutputStream complete");
		outputStream.write(request);
		DebugLog("Write data complete");
		outputStream.close();

		int responseCode = operation.getResponseCode();
		if (responseCode != ResponseCodes.OBEX_HTTP_CONTINUE && responseCode != ResponseCodes.OBEX_HTTP_OK)
		{
			ErrorLog("responseCode = " + responseCode);
			ErrorLog("responseCode != ResponseCodes.OBEX_HTTP_CONTINUE && responseCode != ResponseCodes.OBEX_HTTP_OK");
			return null;
		}

		int length = (int) operation.getLength();
		DebugLog("length = " + length);
		if (length <= 0)
		{
			ErrorLog("length <= 0");
			return null;
		}

		InputStream inputStream = operation.openInputStream();
		DebugLog("Open InputStream complete");
		byte[] response = new byte[length];
		inputStream.read(response);
		inputStream.close();

		DebugLog("Response Content = \n" + new String(response));

		return response;
	}

	public byte[] getPrinterServiceUuid(UUID uuidTransport, byte[] uuidService)
	{
		if (mTransport.getPrinterUuid() == uuidTransport)
		{
			return null;
		}

		return uuidService;
	}

	public byte[] SendSoapRequest(byte[] request)
	{
		HeaderSet reqHeaderSet = new HeaderSet();
		reqHeaderSet.setHeader(HeaderSet.TYPE, "x-obex/bt-SOAP");

		byte[] uuid = getPrinterServiceUuid(BppObexTransport.uuidDirectPrinting, UUID_DPS);

		try
		{
			byte[] response = GetByteArray(request, reqHeaderSet, uuid);

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
		ErrorLog("BluetoothPrinterRun No implementation");

		return false;
	}

	public Message BuildMessage(int what, int arg1, Bundle data)
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

	public void SendMessage(int what, int arg1)
	{
		BuildMessage(what, arg1, null).sendToTarget();
	}

	public void setProgressMessage(String message)
	{
		SendMessage(BPP_MSG_SET_PROGRESS_MESSAGE, 0, message);
	}

	public void setProgressTitle(String message)
	{
		SendMessage(BPP_MSG_SET_PROGRESS_TITLE, 0, message);
	}

	public void SendMessageDelay(int what, int arg1, Bundle data, long delayMillis)
	{
		mHandler.sendMessageDelayed(BuildMessage(what, arg1, data), delayMillis);
	}

	public void SendMessage(int what, int arg1, String message)
	{
		Bundle bundle;

		if (message != null)
		{
			bundle = new Bundle();
			bundle.putString("message", message);
		}
		else
		{
			bundle = null;
		}

		BuildMessage(what, arg1, bundle).sendToTarget();
	}

	public boolean WaitPrinterReady(long interval)
	{
		setProgressMessage("Wait Printer Ready");

		BluetoothPrinterAttribute attribute = new BluetoothPrinterAttribute(this);
		attribute.setBody(attribute.buildBody("PrinterState", "PrinterStateReasons"));

		for (int i = 0; i < 50; i++)
		{
			try
			{
				sleep(interval);
			}
			catch (InterruptedException e)
			{
				e.printStackTrace();
			}

			if (attribute.updateSimple() == false)
			{
				continue;
			}

			String state = attribute.getPrinterState();
			String reasons = attribute.getPrinterStateReasons();

			setProgressMessage(String.format("PrinterState = %s\nPrinterStateReasons = %s\ncount = %d", state, reasons, i));

			if (state.equals("processing"))
			{
				continue;
			}

			if (state.equals("idle"))
			{
				CavanLog("Printer is ready");
				return true;
			}
			else
			{
				CavanLog("Printer is stoped");
				return false;
			}
		}

		return false;
	}

	@Override
	public void run()
	{
		// Process.setThreadPriority(Process.THREAD_PRIORITY_BACKGROUND);

		DebugLog("Bpp Soap request running");

		mWakeLock.acquire();

		try
		{
			DebugLog("Connect to printer");
			mTransport.connect();
		}
		catch (IOException e1)
		{
			e1.printStackTrace();
			SendMessage(BPP_MSG_CONNECT_FAILED, -1);
			return;
		}

		if (BluetoothPrinterRun())
		{
			DebugLog("Print complete");
		}
		else
		{
			DebugLog("Print failed");
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
