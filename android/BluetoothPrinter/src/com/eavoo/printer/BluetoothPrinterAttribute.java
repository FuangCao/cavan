package com.eavoo.printer;

import java.io.IOException;
import java.io.Serializable;
import java.util.List;

import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Element;
import org.xml.sax.SAXException;

public class BluetoothPrinterAttribute extends BppSoapRequest implements Serializable
{
	private static final long serialVersionUID = 1L;

	public BluetoothPrinterAttribute(BluetoothBasePrinter printer)
	{
		super(printer);
		setAttributes("GetPrinterAttributes", null, null);
	}

	private String mPrinterName;
	private String mPrinterLocation;
	private String mPrinterState;
	private String mPrinterStateReasons;
	private int mMaxCopiesSupported;
	private int mNumberUpSupported;
	private int mQueuedJobCount;
	private int mBasicTextPageWidth;
	private int mBasicTextPageHeight;
	private int mOperationStatus;
	private boolean mColorSupported;
	private List<String> mDocumentFormatsSupported;
	private List<String> mSidesSupported;
	private List<String> mOrientationsSupported;
	private List<String> mMediaSizesSupported;
	private List<String> mMediaTypesSupported;
	private List<String> mPrintQualitySupported;
	private List<String> mImageFormatsSupported;
	private String mPrinterGeneralCurrentOperator;

	public String getPrinterName()
	{
		return mPrinterName;
	}

	public void setPrinterName(String mPrinterName)
	{
		this.mPrinterName = mPrinterName;
	}

	public String getPrinterLocation()
	{
		return mPrinterLocation;
	}

	public void setPrinterLocation(String mPrinterLocation)
	{
		this.mPrinterLocation = mPrinterLocation;
	}

	public String getPrinterState()
	{
		return mPrinterState;
	}

	public void setPrinterState(String mPrinterState)
	{
		this.mPrinterState = mPrinterState;
	}

	public String getPrinterStateReasons()
	{
		return mPrinterStateReasons;
	}

	public void setPrinterStateReasons(String mPrinterStateReasons)
	{
		this.mPrinterStateReasons = mPrinterStateReasons;
	}

	public int getMaxCopiesSupported()
	{
		return mMaxCopiesSupported;
	}

	public void setMaxCopiesSupported(int mMaxCopiesSupported)
	{
		this.mMaxCopiesSupported = mMaxCopiesSupported;
	}

	public int getNumberUpSupported()
	{
		return mNumberUpSupported;
	}

	public void setNumberUpSupported(int mNumberUpSupported)
	{
		this.mNumberUpSupported = mNumberUpSupported;
	}

	public int getQueuedJobCount()
	{
		return mQueuedJobCount;
	}

	public void setQueuedJobCount(int mQueuedJobCount)
	{
		this.mQueuedJobCount = mQueuedJobCount;
	}

	public int getBasicTextPageWidth()
	{
		return mBasicTextPageWidth;
	}

	public void setBasicTextPageWidth(int mBasicTextPageWidth)
	{
		this.mBasicTextPageWidth = mBasicTextPageWidth;
	}

	public int getBasicTextPageHeight()
	{
		return mBasicTextPageHeight;
	}

	public void setBasicTextPageHeight(int mBasicTextPageHeight)
	{
		this.mBasicTextPageHeight = mBasicTextPageHeight;
	}

	public int getOperationStatus()
	{
		return mOperationStatus;
	}

	public void setOperationStatus(int mOperationStatus)
	{
		this.mOperationStatus = mOperationStatus;
	}

	public boolean ismColorSupported()
	{
		return mColorSupported;
	}

	public void setColorSupported(boolean mColorSupported)
	{
		this.mColorSupported = mColorSupported;
	}

	public List<String> getDocumentFormatsSupported()
	{
		return mDocumentFormatsSupported;
	}

	public void setDocumentFormatsSupported(List<String> mDocumentFormatsSupported)
	{
		this.mDocumentFormatsSupported = mDocumentFormatsSupported;
	}

	public List<String> getSidesSupported()
	{
		return mSidesSupported;
	}

	public void setSidesSupported(List<String> mSidesSupported)
	{
		this.mSidesSupported = mSidesSupported;
	}

	public List<String> getOrientationsSupported()
	{
		return mOrientationsSupported;
	}

	public void setOrientationsSupported(List<String> mOrientationsSupported)
	{
		this.mOrientationsSupported = mOrientationsSupported;
	}

	public List<String> getMediaSizesSupported()
	{
		return mMediaSizesSupported;
	}

	public void setMediaSizesSupported(List<String> mMediaSizesSupported)
	{
		this.mMediaSizesSupported = mMediaSizesSupported;
	}

	public List<String> getMediaTypesSupported()
	{
		return mMediaTypesSupported;
	}

	public void setMediaTypesSupported(List<String> mMediaTypesSupported)
	{
		this.mMediaTypesSupported = mMediaTypesSupported;
	}

	public List<String> getPrintQualitySupported()
	{
		return mPrintQualitySupported;
	}

	public void setPrintQualitySupported(List<String> mPrintQualitySupported)
	{
		this.mPrintQualitySupported = mPrintQualitySupported;
	}

	public List<String> getImageFormatsSupported()
	{
		return mImageFormatsSupported;
	}

	public void setImageFormatsSupported(List<String> mImageFormatsSupported)
	{
		this.mImageFormatsSupported = mImageFormatsSupported;
	}

	public String getPrinterGeneralCurrentOperator()
	{
		return mPrinterGeneralCurrentOperator;
	}

	public void setPrinterGeneralCurrentOperator(String mPrinterGeneralCurrentOperator)
	{
		this.mPrinterGeneralCurrentOperator = mPrinterGeneralCurrentOperator;
	}

	public boolean parse(Element xml)
	{
		if (xml == null)
		{
			return false;
		}

		mPrinterName = getElementContent(xml, "PrinterName", mPrinterName);
		mPrinterLocation = getElementContent(xml, "PrinterLocation", mPrinterLocation);
		mPrinterState = getElementContent(xml, "PrinterState", mPrinterState);
		mPrinterStateReasons = getElementContent(xml, "PrinterStateReasons", mPrinterStateReasons);
		mDocumentFormatsSupported = getElementContents(xml, "DocumentFormatsSupported", "DocumentFormat", mDocumentFormatsSupported);
		mColorSupported = getElementContentBoolean(xml, "ColorSupported", mColorSupported);
		mMaxCopiesSupported = getElementContentInt(xml, "MaxCopiesSupported", mMaxCopiesSupported);
		mSidesSupported = getElementContents(xml, "SidesSupported", "SidesSupported", mSidesSupported);
		mNumberUpSupported = getElementContentInt(xml, "NumberUpSupported", mNumberUpSupported);
		mOrientationsSupported = getElementContents(xml, "OrientationsSupported", "Orientation", mOrientationsSupported);
		mMediaSizesSupported = getElementContents(xml, "MediaSizesSupported", "MediaSize", mMediaSizesSupported);
		mMediaTypesSupported = getElementContents(xml, "MediaTypesSupported", "MediaType", mMediaTypesSupported);
		mPrintQualitySupported = getElementContents(xml, "PrintQualitySupported", "PrintQuality", mPrintQualitySupported);
		mQueuedJobCount = getElementContentInt(xml, "QueuedJobCount", mQueuedJobCount);
		mImageFormatsSupported = getElementContents(xml, "ImageFormatsSupported", "ImageFormat", mImageFormatsSupported);
		mBasicTextPageWidth = getElementContentInt(xml, "BasicTextPageWidth", mBasicTextPageWidth);
		mBasicTextPageHeight = getElementContentInt(xml, "BasicTextPageHeight", mBasicTextPageHeight);
		mPrinterGeneralCurrentOperator = getElementContent(xml, "PrinterGeneralCurrentOperator", mPrinterGeneralCurrentOperator);
		mOperationStatus = getElementContentInt(xml, "OperationStatus", mOperationStatus);

		return true;
	}

	public String buildBody(String ... attrs)
	{
		if (attrs == null || attrs.length <= 0)
		{
			return null;
		}

		StringBuilder builder = new StringBuilder();
		builder.append("<RequestedPrinterAttributes>\r\n");

		for (String string : attrs)
		{
			builder.append("<PrinterAttribute>" + string + "</PrinterAttribute>\r\n");
		}

		builder.append("</RequestedPrinterAttributes>");

		return builder.toString();
	}

	public boolean updateSimple()
	{
		if (SendTo() == false)
		{
			return false;
		}

		Element element;
		try
		{
			element = ParseSoapResponse("GetPrinterAttributesResponse");
		}
		catch (ParserConfigurationException e)
		{
			e.printStackTrace();
			return false;
		}
		catch (SAXException e)
		{
			e.printStackTrace();
			return false;
		}
		catch (IOException e)
		{
			e.printStackTrace();
			return false;
		}

		if (element == null)
		{
			return false;
		}

		return parse(element);
	}

	public boolean update(String ... attrs)
	{
		setBody(buildBody(attrs));
		return updateSimple();
	}
}
