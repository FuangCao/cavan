package com.eavoo.printer;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;

import javax.obex.ClientSession;
import javax.obex.HeaderSet;
import javax.obex.ResponseCodes;

import com.sun.pdfview.PDFFile;
import com.sun.pdfview.PDFPage;

import android.content.Context;
import android.os.PowerManager;
import android.os.Process;
import android.os.PowerManager.WakeLock;
import android.util.Log;
import android.webkit.MimeTypeMap;

public class BppBase extends Thread
{
	private static final String TAG = "BppBase";
	private byte[] mUuid;
	protected BppObexTransport mTransport;
	protected ClientSession mObexClientSession;
	protected WakeLock mWakeLock;
	protected Context mContext;
	// protected String mFilePath = "/mnt/sdcard/test.jpg";
	// protected String mFilePath = "/mnt/sdcard/BPP_SPEC_V12r00.pdf";
	protected String mFilePath = "/mnt/sdcard/printer.xml";
	protected String mFileType = "text/plain";

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

	public void CavanLog(String message)
	{
		Log.v("Cavan", "\033[1m" + message + "\033[0m");
	}

	public void CavanLog(byte[] bs)
	{
		CavanLog(ByteArrayToHexString(bs));
	}

	public BppBase(Context context, BppObexTransport transport, byte[] uuid)
	{
		CavanLog("Create PrintThread");
		PowerManager pm = (PowerManager)context.getSystemService(Context.POWER_SERVICE);
		this.mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, TAG);

		this.mContext = context;
		this.mTransport = transport;
		this.mUuid = uuid;
	}

	public boolean connect() throws IOException
	{
		CavanLog("Create ClientSession with transport " + mTransport.toString());

		CavanLog("Connect to printer");
		mTransport.connect();
		CavanLog("Create OBEX client session");
		mObexClientSession = new ClientSession(mTransport);
		if (mObexClientSession == null)
		{
			CavanLog("mObexClientSession == null");
			return false;
		}

		HeaderSet hsRequest = new HeaderSet();

		if (mUuid != null)
		{
			hsRequest.setHeader(HeaderSet.TARGET, mUuid);
		}

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

		mTransport.close();

		return false;
	}

	public byte[] getmUuid()
	{
		return mUuid;
	}

	public void setmUuid(byte[] mUuid)
	{
		this.mUuid = mUuid;
	}

	public void disconnect() throws IOException
	{
		CavanLog("disconnect");

		if (mObexClientSession != null)
		{
			mObexClientSession.disconnect(null);
			mObexClientSession.close();
		}

		if (mTransport != null)
		{
			mTransport.close();
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

	public boolean BppObexRun()
	{
		CavanLog("BppObexRun No implementation");

		return false;
	}

	@Override
	public void run()
	{
		Process.setThreadPriority(Process.THREAD_PRIORITY_BACKGROUND);

		CavanLog("Bpp Soap request running");

		mWakeLock.acquire();

		try
		{
			if (connect())
			{
				CavanLog("Connect successfully");
			}
			else
			{
				CavanLog("Connect failed");
				return;
			}
		}
		catch (IOException e)
		{
			// TODO Auto-generated catch block
			e.printStackTrace();
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
			disconnect();
		}
		catch (IOException e)
		{
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		mWakeLock.release();
	}
}
