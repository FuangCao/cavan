// HuaComm.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "HuaComm.h"
#include "HuaCommDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHuaCommApp

BEGIN_MESSAGE_MAP(CHuaCommApp, CWinApp)
	//{{AFX_MSG_MAP(CHuaCommApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHuaCommApp construction

CHuaCommApp::CHuaCommApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CHuaCommApp object

CHuaCommApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CHuaCommApp initialization

BOOL CHuaCommApp::InitInstance()
{
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

	CHuaCommDlg dlg;
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

DWORD HuaComm::PollThread(LPVOID pvarg)
{
	((HuaComm *)pvarg)->Poll();

	return 0;
}

HuaComm::HuaComm(int port)
{
	m_nCommPort = port;
	m_hComm = INVALID_HANDLE_VALUE;
	m_hMutex = INVALID_HANDLE_VALUE;
	m_hRdEvent = INVALID_HANDLE_VALUE;
	m_hWrEvent = INVALID_HANDLE_VALUE;
	m_hRdThread = INVALID_HANDLE_VALUE;
}

bool HuaComm::IsOpen(void)
{
	return m_hComm != INVALID_HANDLE_VALUE;
}

bool HuaComm::Open(void)
{
	if (IsOpen())
	{
		Close();
	}

	m_hMutex = CreateMutex(NULL, FALSE, "HuaComm");
	if (m_hMutex == INVALID_HANDLE_VALUE)
	{
		Close();
		return false;
	}

	m_hRdEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_hRdEvent == INVALID_HANDLE_VALUE)
	{
		Close();
		return false;
	}

	m_hWrEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	if (m_hWrEvent == INVALID_HANDLE_VALUE)
	{
		Close();
		return false;
	}

	char name[1024];
	_snprintf(name, sizeof(name), "COM%d", m_nCommPort);

	m_hComm = CreateFile(name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		Close();
		return false;
	}

	m_hRdThread = CreateThread(NULL, 0, HuaComm::PollThread, this, 0, NULL);
	if (m_hRdThread == INVALID_HANDLE_VALUE)
	{
		Close();
		return false;
	}

	return true;
}

void HuaComm::Close(void)
{
	if (m_hRdThread != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hRdThread);
		m_hRdThread = INVALID_HANDLE_VALUE;
	}

	if (m_hComm != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hComm);
		m_hComm = INVALID_HANDLE_VALUE;
	}

	if (m_hMutex != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hMutex);
		m_hMutex = INVALID_HANDLE_VALUE;
	}

	if (m_hRdEvent != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hRdEvent);
		m_hRdEvent = INVALID_HANDLE_VALUE;
	}

	if (m_hWrEvent != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hWrEvent);
		m_hWrEvent = INVALID_HANDLE_VALUE;
	}
}

void HuaComm::Poll(void)
{
	WaitForSingleObject(m_hMutex, INFINITE);

	while (1)
	{
		DWORD rLen;

		while (1)
		{
			if (tail < head)
			{
				rLen = head - tail - 1;
			}
			else
			{
				rLen = last - tail;
			}

			if (rLen > 0)
			{
				break;
			}

			ReleaseMutex(m_hMutex);
			WaitForSingleObject(m_hWrEvent, INFINITE);
			WaitForSingleObject(m_hMutex, INFINITE);
		}

		DWORD rdLen;
		char *p = tail;

		ReleaseMutex(m_hMutex);
		BOOL res = ReadFile(m_hComm, p, rLen, &rdLen, NULL);
		WaitForSingleObject(m_hMutex, INFINITE);

		if (res == FALSE)
		{
			break;
		}

		tail += rdLen;
		if (tail == last)
		{
			tail = m_RdDataPool;
		}

		if (rdLen > 0)
		{
			PulseEvent(m_hRdEvent);
		}
	}

	ReleaseMutex(m_hMutex);
}

int HuaComm::Read(void *buff, size_t size, int timeout)
{
	int rdLen;

	while (1)
	{
		WaitForSingleObject(m_hMutex, INFINITE);

		while (head == tail)
		{
			ReleaseMutex(m_hMutex);

			if (timeout > 0)
			{
				if (WaitForSingleObject(m_hRdEvent, timeout) != WAIT_OBJECT_0)
				{
					return -1;
				}
			}
			else
			{
				WaitForSingleObject(m_hRdEvent, INFINITE);
			}

			WaitForSingleObject(m_hMutex, INFINITE);
		}

		if (head < tail)
		{
			rdLen = tail - head;
			if (rdLen > (int)size)
			{
				rdLen = size;
			}

			memcpy(buff, head, tail - head);
			head += rdLen;
		}
		else
		{
			rdLen = last - head;
			if ((int)size < rdLen)
			{
				memcpy(buff, head, size);
				head += rdLen;
			}
			else
			{
				memcpy(buff, head, rdLen);
				buff = (char *)buff + rdLen;
				size -= rdLen;

				int lLen = tail - m_RdDataPool - 1;
				if (lLen > (int)size)
				{
					lLen = size;
				}

				if (lLen > 0)
				{
					memcpy(buff, m_RdDataPool, lLen);
					rdLen += lLen;
					head = m_RdDataPool + lLen;
				}
			}
		}

		ReleaseMutex(m_hMutex);
	}

	return rdLen;
}

int HuaComm::Write(const void *buff, size_t size)
{
	DWORD wrLen;

	if (WriteFile(m_hComm, buff, size, &wrLen, NULL) == FALSE)
	{
		return -1;
	}

	return wrLen;
}
