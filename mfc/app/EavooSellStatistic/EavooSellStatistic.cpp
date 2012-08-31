// EavooSellStatistic.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "EavooSellStatistic.h"
#include "EavooSellStatisticDlg.h"
#include "EavooShortMessage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEavooSellStatisticApp

BEGIN_MESSAGE_MAP(CEavooSellStatisticApp, CWinApp)
	//{{AFX_MSG_MAP(CEavooSellStatisticApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEavooSellStatisticApp construction

CEavooSellStatisticApp::CEavooSellStatisticApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance

	if (GetCurrentDirectory(sizeof(mDatabasePath), mDatabasePath) < 0)
	{
		strcpy(mDatabasePath, DEFAULT_CACHE_FILENAME);
	}
	else
	{
		strcat(mDatabasePath, "\\" DEFAULT_CACHE_FILENAME);
	}
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CEavooSellStatisticApp object

CEavooSellStatisticApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CEavooSellStatisticApp initialization

BOOL CEavooSellStatisticApp::InitInstance()
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

	CEavooSellStatisticDlg dlg;
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

bool CEavooSellStatisticApp::OpenDatabase(CFile &file, const char *pathname, UINT nOpenFlags)
{
	if (pathname == NULL)
	{
		pathname = mDatabasePath;
	}

	if (file.Open(pathname, nOpenFlags, NULL) == false)
	{
		return false;
	}

	if ((nOpenFlags & (CFile::modeWrite | CFile::modeReadWrite)) && (nOpenFlags & CFile::modeNoTruncate))
	{
		file.SeekToEnd();
	}

	return true;
}

DWORD CEavooSellStatisticApp::WriteDatabase(CFile &file, const char *buff, DWORD length)
{
	DWORD dwWrite;
	CSingleLock lock(&mDatabaseMutex);

	lock.Lock();

	if (::WriteFile((HANDLE)file.m_hFile, buff, length, &dwWrite, NULL) == FALSE || dwWrite != length || ::FlushFileBuffers((HANDLE)file.m_hFile) == FALSE)
	{
		file.Close();
		dwWrite = 0;
	}

	return dwWrite;
}

DWORD CEavooSellStatisticApp::ReadDatabase(CFile &file, char *buff, DWORD length)
{
	DWORD dwRead;
	CSingleLock lock(&mDatabaseMutex);

	lock.Lock();

	if (::ReadFile((HANDLE)file.m_hFile, buff, length, &dwRead, NULL) == FALSE || dwRead != length)
	{
		file.Close();
		dwRead = 0;
	}

	return dwRead;
}

DWORD CEavooSellStatisticApp::ReadDatabaseNolock(CFile &file, char *buff, DWORD length)
{
	DWORD dwRead;

	if (::ReadFile((HANDLE)file.m_hFile, buff, length, &dwRead, NULL) == FALSE || dwRead != length)
	{
		file.Close();
		dwRead = 0;
	}

	return dwRead;
}

DWORD CEavooSellStatisticApp::ReadDatabaseText(CFile &file, char *buff, int size)
{
	int length;
	DWORD rdTotal;

	rdTotal = ReadDatabaseNolock(file, (char *)&length, sizeof(length));
	if (rdTotal == 0 || length >= size)
	{
		return 0;
	}

	if (ReadDatabaseNolock(file, buff, length) == 0)
	{
		return 0;
	}

	buff[length] = 0;

	return rdTotal + length;
}

DWORD CEavooSellStatisticApp::ReadDatabaseOld(CFile &file, CEavooShortMessage &message)
{
	char type;
	DWORD rdTotal, rdLength;
	CSingleLock lock(&mDatabaseMutex);

	rdTotal = 0;
	message.Initialize();
	lock.Lock();

	while (1)
	{
		rdLength = ReadDatabaseNolock(file, &type, 1);
		if (rdLength== 0)
		{
			return 0;
		}

		rdTotal += rdLength;

		switch (type)
		{
		case SMS_TYPE_END:
			if (message.IsValid())
			{
				return rdTotal;
			}
			return 0;

		case SMS_TYPE_DATE:
			rdLength = ReadDatabaseNolock(file, (char *)&message.mDate, sizeof(message.mDate));
			if (rdLength == 0)
			{
				return 0;
			}
			rdTotal += rdLength;
			break;

		case SMS_TYPE_ADDRESS:
			rdLength = ReadDatabaseText(file, message.mAddress, sizeof(message.mAddress));
			if (rdLength == 0)
			{
				return 0;
			}
			rdTotal += rdLength;
			break;

		case SMS_TYPE_BODY:
			rdLength = ReadDatabaseText(file, message.mBody, sizeof(message.mBody));
			if (rdLength == 0)
			{
				return 0;
			}
			rdTotal += rdLength;
			break;
		}
	}
}