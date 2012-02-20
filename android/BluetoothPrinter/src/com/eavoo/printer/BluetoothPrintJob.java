package com.eavoo.printer;

import java.util.HashMap;

import javax.obex.ApplicationParameter;
import javax.obex.HeaderSet;

import android.webkit.MimeTypeMap;

public class BluetoothPrintJob
{
	private static final byte AppTagOffset = 1;
	private static final byte AppTagCount = 2;
	private static final byte AppTagJobId = 3;
	private static final byte AppTagFileSize = 4;

	private int mCopies = 1;
	private int mNumberUp = 1;
	private int mJobId;
	private short mOperationStatus;
	private boolean mCancelOnLostLink = true;
	private String mFileName;
	private String mJobName = "MyJob";
	private String mJobOriginatingUserName = "mailto:MyEmail";
	private String mDocumentFormat;
	private String mSides = "one-sided";
	private String mOrientationRequested = "portrait";
	private String mMediaSize = "iso_a4_105x148mm";
	private String mMediaType = "envelope";
	private String mPrintQuality = "high";
	private BluetoothBasePrinter mPrinter;

	public void setPrinter(BluetoothBasePrinter printer)
	{
		this.mPrinter = printer;
	}

	public String getFileName()
	{
		return mFileName;
	}

	public void setFileName(String mFileName)
	{
		this.mFileName = mFileName;
	}

	public int getCopies()
	{
		return mCopies;
	}

	public void setCopies(int mCopies)
	{
		this.mCopies = mCopies;
	}

	public int getNumberUp()
	{
		return mNumberUp;
	}

	public void setNumberUp(int mNumberUp)
	{
		this.mNumberUp = mNumberUp;
	}

	public int getJobId()
	{
		return mJobId;
	}

	public void setJobId(int mJobId)
	{
		this.mJobId = mJobId;
	}

	public short getOperationStatus()
	{
		return mOperationStatus;
	}

	public void setOperationStatus(short mOperationStatus)
	{
		this.mOperationStatus = mOperationStatus;
	}

	public boolean ismCancelOnLostLink()
	{
		return mCancelOnLostLink;
	}

	public void setCancelOnLostLink(boolean mCancelOnLostLink)
	{
		this.mCancelOnLostLink = mCancelOnLostLink;
	}

	public String getJobName()
	{
		return mJobName;
	}

	public void setJobName(String mJobName)
	{
		this.mJobName = mJobName;
	}

	public String getJobOriginatingUserName()
	{
		return mJobOriginatingUserName;
	}

	public void setJobOriginatingUserName(String mJobOriginatingUserName)
	{
		this.mJobOriginatingUserName = mJobOriginatingUserName;
	}

	public String getDocumentFormat()
	{
		if (mDocumentFormat == null)
		{
			return GetFileMimeTypeByName(mFileName);
		}

		return mDocumentFormat;
	}

	public void setDocumentFormat(String mDocumentFormat)
	{
		if (mDocumentFormat == null)
		{
			this.mDocumentFormat = GetFileMimeTypeByName(mFileName);
		}
		else
		{
			this.mDocumentFormat = mDocumentFormat;
		}
	}

	public String getSides()
	{
		return mSides;
	}

	public void setSides(String mSides)
	{
		this.mSides = mSides;
	}

	public String getOrientationRequested()
	{
		return mOrientationRequested;
	}

	public void setOrientationRequested(String mOrientationRequested)
	{
		this.mOrientationRequested = mOrientationRequested;
	}

	public String getMediaSize()
	{
		return mMediaSize;
	}

	public void setMediaSize(String mMediaSize)
	{
		this.mMediaSize = mMediaSize;
	}

	public String getMediaType()
	{
		return mMediaType;
	}

	public void setMediaType(String mMediaType)
	{
		this.mMediaType = mMediaType;
	}

	public String getPrintQuality()
	{
		return mPrintQuality;
	}

	public void setPrintQuality(String mPrintQuality)
	{
		this.mPrintQuality = mPrintQuality;
	}

	public static byte getApptagoffset()
	{
		return AppTagOffset;
	}

	public static byte getApptagcount()
	{
		return AppTagCount;
	}

	public static byte getApptagjobid()
	{
		return AppTagJobId;
	}

	public static byte getApptagfilesize()
	{
		return AppTagFileSize;
	}
	
	public String toString()
	{
		StringBuilder builder = new StringBuilder();

		builder.append("<Copies>" + mCopies + "</Copies>\r\n");
		builder.append("<NumberUp>" + mNumberUp + "</NumberUp>\r\n");
		builder.append("<CancelOnLostLink>" + mCancelOnLostLink + "</CancelOnLostLink>\r\n");
		builder.append("<JobName>" + mJobName + "</JobName>\r\n");
		builder.append("<JobOriginatingUserName>" + mJobOriginatingUserName + "</JobOriginatingUserName>\r\n");
		builder.append("<DocumentFormat>" + getDocumentFormat() + "</DocumentFormat>\r\n");
		builder.append("<Sides>" + mSides + "</Sides>\r\n");
		builder.append("<OrientationRequested>" + mOrientationRequested + "</OrientationRequested>\r\n");
		builder.append("<MediaSize>" + mMediaSize + "</MediaSize>\r\n");
		builder.append("<MediaType>" + mMediaType + "</MediaType>\r\n");
		builder.append("<PrintQuality>" + mPrintQuality + "</PrintQuality>");

		return builder.toString();
	}

	public static byte[] IntegerToByteArray(int value)
	{
		byte[] bs =
		{
			(byte) ((value >> 24) & 0xFF),
			(byte) ((value >> 16) & 0xFF),
			(byte) ((value >> 8) & 0xFF),
			(byte) (value & 0xFF),
		};

		return bs;
	}

	public byte[] buildApplicationParameter()
	{
		ApplicationParameter parameter = new ApplicationParameter();
		parameter.addAPPHeader(AppTagJobId, (byte) 4, IntegerToByteArray(mJobId));

		return parameter.getAPPparam();
	}

	public byte[] buildApplicationParameter(int offset, int count, int size)
	{
		ApplicationParameter parameter = new ApplicationParameter();

		parameter.addAPPHeader(AppTagOffset, (byte) 4, IntegerToByteArray(offset));
		parameter.addAPPHeader(AppTagCount, (byte) 4, IntegerToByteArray(count));
		parameter.addAPPHeader(AppTagJobId, (byte) 4, IntegerToByteArray(mJobId));
		parameter.addAPPHeader(AppTagFileSize, (byte) 4, IntegerToByteArray(size));

		return parameter.getAPPparam();
	}

	public HeaderSet buildHeaderSet(int offset, int count, int size)
	{
		HeaderSet headerSet = new HeaderSet();
		headerSet.setHeader(HeaderSet.APPLICATION_PARAMETER, buildApplicationParameter(offset, count, size));

		return headerSet;
	}

	public HeaderSet buildHeaderSet()
	{
		HeaderSet headerSet = new HeaderSet();
		headerSet.setHeader(HeaderSet.APPLICATION_PARAMETER, buildApplicationParameter());

		return headerSet;
	}

	public boolean create(BluetoothBasePrinter printer)
	{
		mPrinter = printer;
		BppSoapRequest request = new BppSoapRequest(mPrinter);
		if (request.SendRequest("CreateJob", toString(), null) == false)
		{
			return false;
		}

		HashMap<String, String> map = request.getResponseAttributes("CreateJobResponse");
		if (map == null)
		{
			return false;
		}
		
		mJobId = Integer.decode(map.get("JobId"));
		mOperationStatus = Short.decode(map.get("OperationStatus"));
		
		return true;
	}
	
	public boolean cancel()
	{
		BppSoapRequest request = new BppSoapRequest(mPrinter);
		if (request.SendRequest("CancelJob", "<JobId>" + mJobId + "</JobId>", null) == false)
		{
			return false;
		}

		return true;
	}

	public HashMap<String, String> getAttributes()
	{
		BppSoapRequest request = new BppSoapRequest(mPrinter);
		if (request.SendRequest("GetJobAttributes", "<JobId>" + mJobId + "</JobId>", null) == false)
		{
			return null;
		}

		return request.getResponseAttributes("GetJobAttributesResponse");
	}

	public static String FileBaseName(String filename)
	{
		int index = filename.lastIndexOf('/');
		if (index < 0)
		{
			return filename;
		}

		return filename.substring(index + 1);
	}

	public static String GetFileExtension(String pathname)
	{
        int dotIndex = pathname.lastIndexOf(".");
        if (dotIndex < 0)
        {
			return "txt";
        }

		return pathname.substring(dotIndex + 1).toLowerCase();
	}

	public static String GetFileMimeTypeByName(String pathname)
	{
        if (pathname == null)
        {
			return null;
        }

		MimeTypeMap map = MimeTypeMap.getSingleton();

        return map.getMimeTypeFromExtension(GetFileExtension(pathname));
	}
}
