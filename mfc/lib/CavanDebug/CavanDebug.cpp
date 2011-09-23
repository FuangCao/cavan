// CavanDebug.cpp: implementation of the CCavanDebug class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <CavanDebug.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCavanDebug::CCavanDebug()
{

}

CCavanDebug::~CCavanDebug()
{

}

void CCavanDebug::ErrorMessage(HWND hWnd, const char *pcFormat, ...)
{
	char buff[1024];
	va_list ap;

	va_start(ap, pcFormat);
	vsprintf(buff, pcFormat, ap);
	va_end(ap);

	MessageBox(hWnd, buff, "Error", MB_ICONERROR);
}

void CCavanDebug::WarningMessage(HWND hWnd, const char *pcFormat, ...)
{
	char buff[1024];
	va_list ap;

	va_start(ap, pcFormat);
	vsprintf(buff, pcFormat, ap);
	va_end(ap);

	MessageBox(hWnd, buff, "Warning", MB_ICONWARNING);
}

void CCavanDebug::RightMessage(HWND hWnd, const char *pcFormat, ...)
{
	char buff[1024];
	va_list ap;

	va_start(ap, pcFormat);
	vsprintf(buff, pcFormat, ap);
	va_end(ap);

	MessageBox(hWnd, buff, "Right", MB_ICONINFORMATION);
}

