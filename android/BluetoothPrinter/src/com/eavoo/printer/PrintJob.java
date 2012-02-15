package com.eavoo.printer;

import java.io.File;
import java.io.IOException;
import java.util.HashMap;

import javax.obex.ApplicationParameter;
import javax.obex.HeaderSet;
import javax.xml.parsers.ParserConfigurationException;
import org.xml.sax.SAXException;

public class PrintJob extends BppSoapRequest
{
	private int mCopies = 1;
	private int mNumberUp = 1;
	private int mJobId;
	private short mOperationStatus;
	private boolean mCancelOnLostLink = true;
	private String mJobName = "MyJob";
	private String mJobOriginatingUserName = "mailto:MyEmail";
	private String mDocumentFormat = "text/plain";
	private String mSides = "one-sided";
	private String mOrientationRequested = "portrait";
	private String mMediaSize = "iso_a4_105x148mm";
	private String mMediaType = "envelope";
	private String mPrintQuality = "high";

	public PrintJob(BppBase base)
	{
		super(base);
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
		return mDocumentFormat;
	}

	public void setDocumentFormat(String mDocumentFormat)
	{
		this.mDocumentFormat = mDocumentFormat;
	}

	public int getCopies()
	{
		return mCopies;
	}

	public void setCopies(int mCopies)
	{
		this.mCopies = mCopies;
	}

	public String getSides()
	{
		return mSides;
	}

	public void setSides(String mSides)
	{
		this.mSides = mSides;
	}

	public int getNumberUp()
	{
		return mNumberUp;
	}

	public void setNumberUp(int mNumberUp)
	{
		this.mNumberUp = mNumberUp;
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

	public boolean ismCancelOnLostLink()
	{
		return mCancelOnLostLink;
	}

	public void setCancelOnLostLink(boolean mCancelOnLostLink)
	{
		this.mCancelOnLostLink = mCancelOnLostLink;
	}

	private String BuildBody()
	{
		StringBuilder builder = new StringBuilder();

		builder.append("<Copies>" + mCopies + "</Copies>\r\n");
		builder.append("<NumberUp>" + mNumberUp + "</NumberUp>\r\n");
		builder.append("<CancelOnLostLink>" + mCancelOnLostLink + "</CancelOnLostLink>\r\n");
		builder.append("<JobName>" + mJobName + "</JobName>\r\n");
		builder.append("<JobOriginatingUserName>" + mJobOriginatingUserName + "</JobOriginatingUserName>\r\n");
		builder.append("<DocumentFormat>" + mDocumentFormat + "</DocumentFormat>\r\n");
		builder.append("<Sides>" + mSides + "</Sides>\r\n");
		builder.append("<OrientationRequested>" + mOrientationRequested + "</OrientationRequested>\r\n");
		builder.append("<MediaSize>" + mMediaSize + "</MediaSize>\r\n");
		builder.append("<MediaType>" + mMediaType + "</MediaType>\r\n");
		builder.append("<PrintQuality>" + mPrintQuality + "</PrintQuality>");

		return builder.toString();
	}

	public boolean CreateJob() throws IOException, ParserConfigurationException, SAXException
	{
		setAttributes("CreateJob", BuildBody(), null);

		if (SendTo() == false)
		{
			return false;
		}

		HashMap<String, String> map = getResponseAttributes("CreateJobResponse");
		if (map == null)
		{
			return false;
		}

		mJobId = Integer.decode(map.get("JobId"));
		mOperationStatus = Short.decode(map.get("OperationStatus"));

		return true;
	}

	public boolean CancelJob()
	{
		setAttributes("CancelJob", "<JobId>" + mJobId + "</JobId>", null);

		return SendTo();
	}

	public HashMap<String, String> GetJobAttributes()
	{
		setAttributes("GetJobAttributes", "<JobId>" + mJobId + "</JobId>", null);

		if (SendTo() == false)
		{
			return null;
		}

		return getResponseAttributes("GetJobAttributesResponse");
	}
}
