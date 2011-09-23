// CavanText.cpp: implementation of the CCavanText class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <CavanText.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCavanText::CCavanText()
{

}

CCavanText::~CCavanText()
{

}

const char *CCavanText::TextToSize(const char *pcText, ULONGLONG &dwValue)
{
	dwValue = 0;

	while (*pcText >= '0' && *pcText <= '9')
	{
		dwValue *= 10;
		dwValue += *pcText++ - '0';
	}

	switch (*pcText)
	{
	case 't':
	case 'T':
		dwValue <<= 40;
		break;
	case 'g':
	case 'G':
		dwValue <<= 30;
		break;
	case 'm':
	case 'M':
		dwValue <<= 20;
		break;
	case 'k':
	case 'K':
		dwValue <<= 10;
		break;
	case 'b':
	case 'B':
		break;
	default:
		return NULL;
	}

	return pcText + 1;
}

ULONGLONG CCavanText::TextToSize(const char *pcText)
{
	ULONGLONG ullSize, dwTemp;

	ullSize = 0;

	while (pcText && *pcText)
	{
		pcText = TextToSize(pcText, dwTemp);
		ullSize += dwTemp;
	}

	return ullSize;
}

ULONGLONG CCavanText::TextToSize(CString &strText)
{
	return TextToSize(strText.GetBuffer(0));
}

ULONGLONG CCavanText::ToSize(void)
{
	return TextToSize(GetBuffer(0));
}

void CCavanText::SizeToText(ULONGLONG ullSize, char *pcText)
{
	if (ullSize == 0)
	{
		strcpy(pcText, "0B");
		return;
	}

	ULONGLONG dwTemp;

	dwTemp = (ullSize >> 40) & 0x03FF;
	if (dwTemp)
	{
		pcText += sprintf(pcText, "%dT", dwTemp);
	}

	dwTemp = (ullSize >> 30) & 0x03FF;
	if (dwTemp)
	{
		pcText += sprintf(pcText, "%dG", dwTemp);
	}

	dwTemp = (ullSize >> 20) & 0x03FF;
	if (dwTemp)
	{
		pcText += sprintf(pcText, "%dM", dwTemp);
	}

	dwTemp = (ullSize >> 10) & 0x03FF;
	if (dwTemp)
	{
		pcText += sprintf(pcText, "%dK", dwTemp);
	}

	dwTemp = ullSize & 0x03FF;
	if (dwTemp)
	{
		sprintf(pcText, "%dB", dwTemp);
	}
	else
	{
		*pcText++ = 'B';
		*pcText = 0;
	}
}

void CCavanText::SizeToText(ULONGLONG ullSize, CString &strText)
{
	SizeToText(ullSize, strText.GetBuffer(100));
}

const CCavanText &CCavanText::operator=(ULONGLONG ullSize)
{
	SizeToText(ullSize, GetBuffer(100));

	return *this;
}

DWORD CCavanText::UnitToSize(char cUnit)
{
	switch (cUnit)
	{
	case 't':
	case 'T':
		return 1L << 40;
	case 'g':
	case 'G':
		return 1L << 30;
	case 'm':
	case 'M':
		return 1L << 20;
	case 'k':
	case 'K':
		return 1L << 10;
	case 'b':
	case 'B':
		return 1L;
	default:
		return 0;
	}

	return 0;
}
