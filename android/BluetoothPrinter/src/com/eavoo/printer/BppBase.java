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
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.Process;
import android.util.Log;
import android.webkit.MimeTypeMap;

public class BppBase extends Thread
{
	private static final String TAG = "BppBase";
	protected BppObexTransport mTransport;
	private WakeLock mWakeLock;
	// private Context mContext;
	private String mFileName;
	private String mFileType;

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

	public void CavanLog(String message)
	{
		Log.v("Cavan", "\033[1m" + message + "\033[0m");
	}

	public void CavanLog(byte[] bs)
	{
		CavanLog(ByteArrayToHexString(bs));
	}

	public BppBase(Context context, BppObexTransport transport, String filename, String filetype)
	{
		CavanLog("Create PrintThread");
		PowerManager pm = (PowerManager)context.getSystemService(Context.POWER_SERVICE);
		this.mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, TAG);

		// this.mContext = context;
		this.mTransport = transport;
		this.mFileName = filename;
		this.mFileType = filetype;
	}

	public String getFileName()
	{
		return mFileName;
	}

	public void setFileName(String mFileName)
	{
		this.mFileName = mFileName;
	}

	public String getFileType()
	{
		return mFileType == null ? GetFileMimeTypeByName(mFileName) : mFileType;
	}

	public void setFileType(String mFileType)
	{
		this.mFileType = mFileType;
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
		CavanLog("Create ClientSession with transport " + mTransport.toString());
		ClientSession session = new ClientSession(mTransport);
		HeaderSet hsRequest = new HeaderSet();

		if (uuid != null)
		{
			hsRequest.setHeader(HeaderSet.TARGET, uuid);
		}

		CavanLog("Connect to OBEX session");
		HeaderSet hsResponse = session.connect(hsRequest);
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

		return hsResponse.getResponseCode() == ResponseCodes.OBEX_HTTP_OK ? session : null;
	}

	public String GetFileExtension(String pathname)
	{
        int dotIndex = pathname.lastIndexOf(".");
        if (dotIndex < 0)
        {
			return "txt";
        }

		return pathname.substring(dotIndex + 1).toLowerCase();
	}

	public String GetFileMimeTypeByName(String pathname)
	{
        if (pathname == null)
        {
			return null;
        }

		MimeTypeMap map = MimeTypeMap.getSingleton();

        return map.getMimeTypeFromExtension(GetFileExtension(pathname));
	}

	public boolean BppObexRun()
	{
		CavanLog("BppObexRun No implementation");

		return false;
	}

	public String FileBaseName(String filename)
	{
		int index = filename.lastIndexOf('/');
		if (index < 0)
		{
			return filename;
		}

		return filename.substring(index + 1);
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
			CavanLog("clientOperation == null");
			return false;
		}

		OutputStream obexOutputStream = clientOperation.openOutputStream();
		if (obexOutputStream == null)
		{
			CavanLog("obexOutputStream == null");
			clientOperation.abort();
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

		BufferedInputStream bufferedInputStream = new BufferedInputStream(inputStream);
		int obexMaxPackageSize = clientOperation.getMaxPacketSize();
		byte[] buff = new byte[obexMaxPackageSize];

		boolean boolResult = true;

		CavanLog("Start send file");

		while (true)
		{
			int readLen = bufferedInputStream.read(buff);
			if (readLen <= 0)
			{
				CavanLog("Send file complete");
				break;
			}

			obexOutputStream.write(buff, 0, readLen);

			int responseCode = clientOperation.getResponseCode();
			if (responseCode != ResponseCodes.OBEX_HTTP_CONTINUE && responseCode != ResponseCodes.OBEX_HTTP_OK)
			{
				CavanLog("responseCode = " + responseCode);
				CavanLog("responseCode != ResponseCodes.OBEX_HTTP_CONTINUE && responseCode != ResponseCodes.OBEX_HTTP_OK");
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
			CavanLog("File " + filename + " don't exist");
			return false;
		}

		if (headerSet == null)
		{
			headerSet = new HeaderSet();
		}

		headerSet.setHeader(HeaderSet.NAME, FileBaseName(filename));
		CavanLog("NAME = " + headerSet.getHeader(HeaderSet.NAME));
		headerSet.setHeader(HeaderSet.LENGTH, size);
		CavanLog("LENGTH = " + headerSet.getHeader(HeaderSet.LENGTH));

		if (filetype == null)
		{
			filetype = GetFileMimeTypeByName(filename);
		}
		headerSet.setHeader(HeaderSet.TYPE, filetype);
		CavanLog("TYPE = " + headerSet.getHeader(HeaderSet.TYPE));

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
		Log.v("GetByteArray", "Request = \n" + new String(request));
		ClientSession session = connect(uuid);
		if (session == null)
		{
			return null;
		}

		ClientOperation operation = (ClientOperation) session.get(headerSet);
		CavanLog("Get operation complete");

		OutputStream outputStream = operation.openOutputStream();
		CavanLog("Open OutputStream complete");
		outputStream.write(request);
		CavanLog("Write data complete");
		outputStream.close();

		int responseCode = operation.getResponseCode();
		if (responseCode != ResponseCodes.OBEX_HTTP_CONTINUE && responseCode != ResponseCodes.OBEX_HTTP_OK)
		{
			CavanLog("responseCode != ResponseCodes.OBEX_HTTP_CONTINUE && responseCode != ResponseCodes.OBEX_HTTP_OK");
			return null;
		}

		int length = (int) operation.getLength();
		CavanLog("length = " + length);
		if (length <= 0)
		{
			return null;
		}

		InputStream inputStream = operation.openInputStream();
		CavanLog("Open InputStream complete");
		byte[] response = new byte[length];
		inputStream.read(response);
		inputStream.close();

		Log.v("Cavan", "Response Content = \n" + new String(response));

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
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		return null;
	}

	@Override
	public void run()
	{
		Process.setThreadPriority(Process.THREAD_PRIORITY_BACKGROUND);

		CavanLog("Bpp Soap request running");

		mWakeLock.acquire();

		try
		{
			CavanLog("Connect to printer");
			mTransport.connect();
		}
		catch (IOException e1)
		{
			// TODO Auto-generated catch block
			e1.printStackTrace();
			return;
		}

		if (BppObexRun())
		{
			CavanLog("Print complete");
		}
		else
		{
			CavanLog("Print failed");
		}

		try
		{
			mTransport.close();
		}
		catch (IOException e)
		{
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		mWakeLock.release();
	}
}
