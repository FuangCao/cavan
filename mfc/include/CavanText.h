// CavanText.h: interface for the CCavanText class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CAVANTEXT_H__E02C2E21_00AE_4C91_9F29_5AFF368BD5CB__INCLUDED_)
#define AFX_CAVANTEXT_H__E02C2E21_00AE_4C91_9F29_5AFF368BD5CB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CCavanText : public CString
{
private:
	static const char *TextToSize(const char *pcText, ULONGLONG &dwValue);
public:
	CCavanText();
	virtual ~CCavanText();
	ULONGLONG ToSize(void);
	static ULONGLONG TextToSize(const char *pcText);
	static ULONGLONG TextToSize(CString &strText);
	const CCavanText &operator=(ULONGLONG ullSize);
	static void SizeToText(ULONGLONG ullSize, char *pcText);
	static void SizeToText(ULONGLONG ullSize, CString &strText);
	static DWORD UnitToSize(char cUnit);
};

#endif // !defined(AFX_CAVANTEXT_H__E02C2E21_00AE_4C91_9F29_5AFF368BD5CB__INCLUDED_)
