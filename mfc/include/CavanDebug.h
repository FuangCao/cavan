// CavanDebug.h: interface for the CCavanDebug class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CAVANDEBUG_H__57C7163F_1B68_4070_9D6A_85449C6DB58C__INCLUDED_)
#define AFX_CAVANDEBUG_H__57C7163F_1B68_4070_9D6A_85449C6DB58C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CCavanDebug
{
public:
	CCavanDebug();
	virtual ~CCavanDebug();
	static void ErrorMessage(HWND hWnd, const char *pcFormat, ...);
	static void WarningMessage(HWND hWnd, const char *pcFormat, ...);
	static void RightMessage(HWND hWnd, const char *pcFormat, ...);
};

#endif // !defined(AFX_CAVANDEBUG_H__57C7163F_1B68_4070_9D6A_85449C6DB58C__INCLUDED_)
