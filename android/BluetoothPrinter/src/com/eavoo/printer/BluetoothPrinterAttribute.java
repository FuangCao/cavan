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

		mPrinterName = getElementContent(xml, "PrinterName");
		mPrinterLocation = getElementContent(xml, "PrinterLocation");
		mPrinterState = getElementContent(xml, "PrinterState");
		mPrinterStateReasons = getElementContent(xml, "PrinterStateReasons");
		mDocumentFormatsSupported = getElementContents(xml, "DocumentFormatsSupported", "DocumentFormat");
		mColorSupported = getElementContentBoolean(xml, "ColorSupported");
		mMaxCopiesSupported = getElementContentInt(xml, "MaxCopiesSupported");
		mSidesSupported = getElementContents(xml, "SidesSupported", "SidesSupported");
		mNumberUpSupported = getElementContentInt(xml, "NumberUpSupported");
		mOrientationsSupported = getElementContents(xml, "OrientationsSupported", "Orientation");
		mMediaSizesSupported = getElementContents(xml, "MediaSizesSupported", "MediaSize");
		mMediaTypesSupported = getElementContents(xml, "MediaTypesSupported", "MediaType");
		mPrintQualitySupported = getElementContents(xml, "PrintQualitySupported", "PrintQuality");
		mQueuedJobCount = getElementContentInt(xml, "QueuedJobCount");
		mImageFormatsSupported = getElementContents(xml, "ImageFormatsSupported", "ImageFormat");
		mBasicTextPageWidth = getElementContentInt(xml, "BasicTextPageWidth");
		mBasicTextPageHeight = getElementContentInt(xml, "BasicTextPageHeight");
		mPrinterGeneralCurrentOperator = getElementContent(xml, "PrinterGeneralCurrentOperator");
		mOperationStatus = getElementContentInt(xml, "OperationStatus");

		return true;
	}

	public boolean update() throws ParserConfigurationException, SAXException, IOException
	{
		setAttributes("GetPrinterAttributes", null, null);
		if (SendTo() == false)
		{
			return false;
		}

		Element element = ParseSoapResponse("GetPrinterAttributesResponse");
		if (element == null)
		{
			return false;
		}

		return parse(element);
	}
}
