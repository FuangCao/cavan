// CavanFile.h: interface for the CCavanFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CAVANFILE_H__EBCD6A54_CAF9_48B0_9435_604BD4DB8089__INCLUDED_)
#define AFX_CAVANFILE_H__EBCD6A54_CAF9_48B0_9435_604BD4DB8089__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxcmn.h>

class CCavanFile : public CFile
{
public:
	CCavanFile();
	virtual ~CCavanFile();

	CString strLog;

	BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags);
	BOOL OpenFormatFile(UINT nOpenFlags, const char *pcFormat, ...);

	BOOL CopyFrom(CProgressCtrl &prgProgress, CFile &fileSrc, LONG lOffIn, LONG lOffOut, DWORD dwLength);
	BOOL CopyFrom(CProgressCtrl &prgProgress, const char *pcFileName, LONG lOffIn, LONG lOffOut, DWORD dwLength);

	static DWORD BufferCRC32(const char *pcBuff, DWORD dwCRC32, size_t stSize);
	BOOL CRC32(CProgressCtrl &prgProgress, DWORD &dwCRC32);
	BOOL CRC32(CProgressCtrl &prgProgress, DWORD &dwCRC32, size_t stSize);
	BOOL CRC32(CProgressCtrl &prgProgress, LONG lOff, UINT nFrom, DWORD &dwCRC32, size_t stSize);
};

#endif // !defined(AFX_CAVANFILE_H__EBCD6A54_CAF9_48B0_9435_604BD4DB8089__INCLUDED_)
