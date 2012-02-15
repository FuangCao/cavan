package com.eavoo.printer;

import java.io.BufferedInputStream;
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
import android.os.Process;
import android.os.PowerManager.WakeLock;
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
		return mFileType == null ?  GetFileMimeTypeByName(mFileName) : mFileType;
	}

	public void setFileType(String mFileType)
	{
		this.mFileType = mFileType;
	}

	/**
	public byte[] PdfToJpeg(int pageIndex) throws IOException
	{
		File file = new File(mFilePath);
		RandomAccessFile randomAccessFile = new RandomAccessFile(file, "r");
		FileChannel channel = randomAccessFile.getChannel();
		ByteBuffer byteBuffer = channel.map(FileChannel.MapMode.READ_ONLY, 0, channel.size());
		PDFFile pdfFile = new PDFFile(byteBuffer);

		PDFPage page = pdfFile.getPage(pageIndex);
		int width = (int) page.getWidth();
		int height = (int) page.getHeight();

		Image image = page.getImage(width, height, new Rectangle(width, height), null, true, true);
		BufferedImage bufferedImage = new BufferedImage(width, height, BufferedImage.TYPE_INT_RGB);
		bufferedImage.getGraphics().drawImage(image, 0, 0, width, height, null);

		ByteArrayOutputStream jpegOutputStream = new ByteArrayOutputStream();
		JPEGImageEncoder encoder = JPEGCodec.createJPEGEncoder(jpegOutputStream);
		encoder.encode(bufferedImage);
		jpegOutputStream.close();

		return jpegOutputStream.toByteArray();
	}
	*/

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

	public void disconnect(ClientSession session) throws IOException
	{
		CavanLog("disconnect");

		session.disconnect(null);
		session.close();
	}

	public String GetFileMimeTypeByName(String pathname)
	{
        String extension;

        if (pathname == null)
        {
			return null;
        }

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

	public boolean BppObexRun()
	{
		CavanLog("BppObexRun No implementation");

		return false;
	}

	public String FileBaseName(String filename)
	{
		int index = mFileName.lastIndexOf('/');
		if (index < 0)
		{
			return filename;
		}

		return filename.substring(index + 1);
	}

	public boolean PutFile(byte[] uuid, HeaderSet headerSet) throws IOException
	{
		if (mFileName == null)
		{
			return false;
		}

		File file = new File(mFileName);
		long fileLength = file.length();

		if (fileLength == 0)
		{
			CavanLog("File " + mFileName + " don't exist");
			return false;
		}

		if (headerSet == null)
		{
			headerSet = new HeaderSet();
		}

		headerSet.setHeader(HeaderSet.NAME, FileBaseName(mFileName));
		CavanLog("NAME = " + headerSet.getHeader(HeaderSet.NAME));
		headerSet.setHeader(HeaderSet.LENGTH, fileLength);
		CavanLog("LENGTH = " + headerSet.getHeader(HeaderSet.LENGTH));
		headerSet.setHeader(HeaderSet.TYPE, getFileType());
		CavanLog("TYPE = " + headerSet.getHeader(HeaderSet.TYPE));


		ClientSession session = connect(uuid);
		if (session == null)
		{
			return false;
		}

		ClientOperation clientOperation = (ClientOperation) session.put(headerSet);
		if (clientOperation == null)
		{
			CavanLog("clientOperation == null");
			disconnect(session);
			return false;
		}

		OutputStream obexOutputStream = clientOperation.openOutputStream();
		if (obexOutputStream == null)
		{
			CavanLog("obexOutputStream == null");
			clientOperation.abort();
			disconnect(session);
			return false;
		}

		InputStream obexInputStream = clientOperation.openInputStream();
		if (obexInputStream == null)
		{
			CavanLog("obexInputStream == null");
			obexOutputStream.close();
			clientOperation.abort();
			disconnect(session);
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

			if (sendLength < 0)
			{
				boolResult = false;
				break;
			}

			obexOutputStream.write(buff, 0, sendLength);

			int responseCode = clientOperation.getResponseCode();
			if (responseCode != ResponseCodes.OBEX_HTTP_CONTINUE && responseCode != ResponseCodes.OBEX_HTTP_OK)
			{
				CavanLog("responseCode = " + responseCode);
				CavanLog("responseCode != ResponseCodes.OBEX_HTTP_CONTINUE && responseCode != ResponseCodes.OBEX_HTTP_OK");
				boolResult = false;
				break;
			}

			fileLength -= sendLength;
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

		disconnect(session);

		return boolResult;
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

		CavanLog("Response Content = \n" + new String(response));

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
