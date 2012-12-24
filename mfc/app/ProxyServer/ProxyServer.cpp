// ProxyServer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "ProxyServer.h"
#include "ProxyServerDlg.h"
#include <errno.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProxyServerApp

BEGIN_MESSAGE_MAP(CProxyServerApp, CWinApp)
	//{{AFX_MSG_MAP(CProxyServerApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProxyServerApp construction

CProxyServerApp::CProxyServerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CProxyServerApp object

CProxyServerApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CProxyServerApp initialization

BOOL CProxyServerApp::InitInstance()
{
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CProxyServerDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

int CavanMessageBox(UINT nType, const char *strFormat, va_list ap)
{
	char buff[1024];

	_vsnprintf(buff, sizeof(buff), strFormat, ap);

	return AfxMessageBox(buff, nType);
}

void CavanMessageBoxError(const char *strFormat, ...)
{
	va_list ap;

	va_start(ap, strFormat);
	CavanMessageBox(MB_ICONERROR, strFormat, ap);
	va_end(ap);
}

void CavanMessageBoxWarning(const char *strFormat, ...)
{
	va_list ap;

	va_start(ap, strFormat);
	CavanMessageBox(MB_ICONWARNING, strFormat, ap);
	va_end(ap);
}

void CavanMessageBoxInfo(const char *strFormat, ...)
{
	va_list ap;

	va_start(ap, strFormat);
	CavanMessageBox(MB_ICONINFORMATION, strFormat, ap);
	va_end(ap);
}

bool CavanMessageBoxYesNo(const char *strFormat, ...)
{
	int ret;
	va_list ap;

	va_start(ap, strFormat);
	ret = CavanMessageBox(MB_ICONQUESTION | MB_YESNO, strFormat, ap);
	va_end(ap);

	return ret == IDYES;
}

int TextLhCmp(const char *left, const char *right)
{
	while (*left)
	{
		if (*left != *right)
		{
			return *left - *right;
		}

		left++, right++;
	}

	return 0;
}

int CharToValue(char c)
{
	if (c >= '0' && c <= '9')
	{
		return c - '0';
	}
	else if (c >= 'a' && c <= 'z')
	{
		return c - 'a' + 10;
	}
	else if (c >= 'A' && c <= 'Z')
	{
		return c - 'A' + 10;
	}
	else
	{
		return -1;
	}
}

DWORD TextToValue(const char *text, int base)
{
	if (base < 1)
	{
		base = 10;
	}

	DWORD dwValue = 0;

	while (1)
	{
		int tmp = CharToValue(*text);
		if (tmp < 0 || tmp >= base)
		{
			break;
		}

		dwValue = dwValue * base + tmp;
		text++;
	}

	return dwValue;
}

bool ExecuteCommand(const char *strCommandName, const char *strArgFormat, ...)
{
	char args[1024];

	if (strArgFormat)
	{
		va_list ap;

		va_start(ap, strArgFormat);
		_vsnprintf(args, sizeof(args), strArgFormat, ap);
		va_end(ap);
	}
	else
	{
		args[0] = 0;
	}

	SHELLEXECUTEINFO ShExecInfo;
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = strCommandName;
	ShExecInfo.lpParameters = args;
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_HIDE;
	ShExecInfo.hInstApp = NULL;

	if (ShellExecuteEx(&ShExecInfo) == false)
	{
		if (GetLastError() != ENOENT)
		{
			return false;
		}

		int ret;
		char buff[1024];

		_snprintf(buff, sizeof(buff), "找不到命令 %s\n\n是否手动选择？", strCommandName);
		ret = AfxMessageBox(buff, MB_ICONQUESTION | MB_YESNO, 0);
		if (ret != IDYES)
		{
			return false;
		}

		_snprintf(buff, sizeof(buff), "%s 文件|%s|可执行文件 (*.exe)|*.exe|所有文件 (*)|*||", strCommandName, strCommandName);
		CFileDialog fileDialog(true, NULL, strCommandName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, buff);
		ret = fileDialog.DoModal();
		if (ret != IDOK)
		{
			return false;
		}

		strcpy(buff, fileDialog.GetPathName());
		ShExecInfo.lpFile = buff;
		if (ShellExecuteEx(&ShExecInfo) == false)
		{
			return false;
		}
	}

	WaitForSingleObject(ShExecInfo.hProcess, INFINITE);

	DWORD dwExitCode;
	bool bRes;

	if (GetExitCodeProcess(ShExecInfo.hProcess, &dwExitCode) && dwExitCode == 0)
	{
		bRes = true;
	}
	else
	{
		bRes = false;
	}

	CloseHandle(ShExecInfo.hProcess);

	return bRes;
}
