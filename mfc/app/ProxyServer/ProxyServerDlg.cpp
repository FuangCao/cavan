// ProxyServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ProxyServer.h"
#include "ProxyServerDlg.h"
#include <process.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define WM_SHOW_TASK	(WM_USER + 1)

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProxyServerDlg dialog

CProxyServerDlg::CProxyServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProxyServerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProxyServerDlg)
	m_nLocalProtocol = 0;
	m_nProxyProtocol = 2;
	m_dwLocalPort = 8888;
	m_dwProxyPort = 8888;
	m_dwDaemonCount = 20;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	mProxyService = new CProxyService(this);
	mServiceRunning = false;
}

void CProxyServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProxyServerDlg)
	DDX_Control(pDX, IDC_RADIO_PROXY_TCP, m_ctrlRadioProxyProtocol);
	DDX_Control(pDX, IDC_STATIC_STATUS, m_ctrlStatus);
	DDX_Control(pDX, IDC_PROGRESS_SERVICE, m_ctrlServiceProgress);
	DDX_Control(pDX, IDC_LIST_SERVICE, m_ctrlListService);
	DDX_Control(pDX, IDC_IPADDRESS_PROXY, m_ctrlProxyIP);
	DDX_Radio(pDX, IDC_RADIO_LOCAL_TCP, m_nLocalProtocol);
	DDX_Radio(pDX, IDC_RADIO_PROXY_TCP, m_nProxyProtocol);
	DDX_Text(pDX, IDC_EDIT_LOCAL_PORT, m_dwLocalPort);
	DDX_Text(pDX, IDC_EDIT_PROXY_PORT, m_dwProxyPort);
	DDX_Text(pDX, IDC_EDIT_DAEMON_COUNT, m_dwDaemonCount);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CProxyServerDlg, CDialog)
	//{{AFX_MSG_MAP(CProxyServerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START, OnButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnButtonStop)
	ON_BN_CLICKED(IDC_RADIO_PROXY_TCP, OnRadioProxyProtocol)
	ON_WM_DESTROY()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_MENUITEM_EXIT, OnMenuitemExit)
	ON_BN_CLICKED(IDC_RADIO_PROXY_UDP, OnRadioProxyProtocol)
	ON_BN_CLICKED(IDC_RADIO_PROXY_ADB, OnRadioProxyProtocol)
	ON_COMMAND(ID_MENUITEM_START, OnButtonStart)
	ON_COMMAND(ID_MENUITEM_STOP, OnButtonStop)
	ON_COMMAND(ID_MENUITEM_VISIBLE, OnMenuitemVisible)
	ON_MESSAGE(WM_SHOW_TASK, OnNotifyIconProxy)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProxyServerDlg message handlers

BOOL CProxyServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_ctrlProxyIP.SetAddress(127, 0, 0, 1);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(false);
	m_ctrlListService.InsertColumn(0, "ID", LVCFMT_LEFT, 50, -1);
	m_ctrlListService.InsertColumn(1, "状态", LVCFMT_LEFT, 50, -1);
	m_ctrlListService.InsertColumn(2, "IP地址", LVCFMT_LEFT, 100, -1);
	m_ctrlListService.InsertColumn(3, "端口号", LVCFMT_LEFT, 60, -1);

	mNotifyIconData.cbSize = sizeof(mNotifyIconData);
	mNotifyIconData.hIcon = m_hIcon;
	mNotifyIconData.hWnd = GetSafeHwnd();
	mNotifyIconData.uID = IDR_MAINFRAME;
	strcpy(mNotifyIconData.szTip, "代理服务器");
	mNotifyIconData.uCallbackMessage = WM_SHOW_TASK;
	mNotifyIconData.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	Shell_NotifyIcon(NIM_ADD, &mNotifyIconData);

	mProxyMenu.LoadMenu(IDR_MENU_PROXY);

	OnRadioProxyProtocol();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CProxyServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	switch (nID & 0xFFF0)
	{
	case IDM_ABOUTBOX:
		{
			CAboutDlg dlgAbout;
			dlgAbout.DoModal();
		}
		break;

	case SC_CLOSE:
		if (mServiceRunning && CavanMessageBoxYesNo("服务器正在运行，是否最小化到托盘？"))
		{
			ShowWindow(SW_HIDE);
			break;
		}

	default:
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CProxyServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CProxyServerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

CProxyProcotolType CProxyServerDlg::ValueToProtocolType(int value)
{
	switch (value)
	{
	case 0:
		return PROXY_PROTOCOL_TYPE_TCP;
	case 1:
		return PROXY_PROTOCOL_TYPE_UDP;
	case 2:
		return PROXY_PROTOCOL_TYPE_ADB;
	default:
		return PROXY_PROTOCOL_TYPE_NONE;
	}
}

void CProxyServerDlg::ShowStatus(const char *strFormat, ...)
{
	char text[1024];
	va_list ap;

	va_start(ap, strFormat);
	_vsnprintf(text, sizeof(text), strFormat, ap);
	va_end(ap);

	m_ctrlStatus.SetWindowText(text);
}

void CProxyServerDlg::EnableAllWindow(bool enable)
{
	int ids[] =
	{
		IDC_EDIT_LOCAL_PORT,
		IDC_EDIT_PROXY_PORT,
		IDC_RADIO_PROXY_TCP,
		IDC_RADIO_PROXY_UDP,
		IDC_RADIO_PROXY_ADB,
		IDC_RADIO_LOCAL_TCP,
		IDC_RADIO_LOCAL_UDP,
		IDC_EDIT_DAEMON_COUNT
	};

	for (int i = 0; i < NELEM(ids); i++)
	{
		GetDlgItem(ids[i])->EnableWindow(enable);
	}

	if (enable)
	{
		GetDlgItem(IDC_IPADDRESS_PROXY)->EnableWindow(m_nProxyProtocol != 2);
		GetDlgItem(IDC_BUTTON_START)->EnableWindow(true);
		GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(false);
	}
	else
	{
		GetDlgItem(IDC_IPADDRESS_PROXY)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_START)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(true);
	}
}

bool CProxyServerDlg::StartAdbServer(void)
{
	ShowStatus("正在启动ADB服务器 请稍等");

	if (ExecuteCommand("adb", "start-server"))
	{
		ShowStatus("启动ADB服务器成功");
		return true;
	}

	ShowStatus("启动ADB服务器成功失败");

	return false;
}

void CProxyServerDlg::StartThreadHandler(void *data)
{
	CProxyServerDlg *dlg = (CProxyServerDlg *)data;
	CButton *btnStart = (CButton *)dlg->GetDlgItem(IDC_BUTTON_START);

	btnStart->EnableWindow(false);

	if (dlg->EnableService(true))
	{
		dlg->EnableAllWindow(false);
	}
	else
	{
		btnStart->EnableWindow(true);
	}
}

void CProxyServerDlg::StopThreadHandler(void *data)
{
	CProxyServerDlg *dlg = (CProxyServerDlg *)data;
	CButton *btnStop = (CButton *)dlg->GetDlgItem(IDC_BUTTON_STOP);

	btnStop->EnableWindow(false);
	dlg->EnableService(false);
	dlg->EnableAllWindow(true);
}

bool CProxyServerDlg::EnableService(bool enable)
{
	const char *strResult;

	if (enable)
	{
		DWORD dwAddress;
		m_ctrlProxyIP.GetAddress(dwAddress);

		if (dwAddress == INADDR_LOOPBACK && m_nLocalProtocol == m_nProxyProtocol && m_dwLocalPort == m_dwProxyPort)
		{
			CavanMessageBoxError("自己不能代理自己");
			return false;
		}

		if (m_nProxyProtocol == 2 && StartAdbServer() == false)
		{
			CavanMessageBoxError("启动ADB服务器失败");
			return false;
		}

		mProxyService->Prepare(m_dwProxyPort, m_dwLocalPort, dwAddress, ValueToProtocolType(m_nLocalProtocol), ValueToProtocolType(m_nProxyProtocol), m_dwDaemonCount);

		if (mProxyService->Start() == false)
		{
			return false;
		}

		mServiceRunning = true;
		strResult = "启动";
	}
	else
	{
		mProxyService->Stop();
		mServiceRunning = false;
		strResult = "停止";
	}

	if (IsWindowVisible() == false)
	{
		CavanMessageBoxInfo("代理服务器已%s", strResult);
	}

	return true;
}

void CProxyServerDlg::OnButtonStart()
{
	UpdateData(true);

	_beginthread(StartThreadHandler, 0, this);
}

void CProxyServerDlg::OnButtonStop()
{
	_beginthread(StopThreadHandler, 0, this);
}

void CProxyServerDlg::OnRadioProxyProtocol()
{
	UpdateData(true);

	if (m_nProxyProtocol == 2)
	{
		m_ctrlProxyIP.EnableWindow(false);
	}
	else
	{
		m_ctrlProxyIP.EnableWindow(true);
	}
}

void CProxyServerDlg::OnDestroy()
{
	CDialog::OnDestroy();

	Shell_NotifyIcon(NIM_DELETE, &mNotifyIconData);
}

LRESULT CProxyServerDlg::OnNotifyIconProxy(WPARAM wParam, LPARAM lParam)
{
	switch (lParam)
	{
	case WM_RBUTTONUP:
		{
			CPoint point;
			GetCursorPos(&point);
			SetForegroundWindow();
			OnContextMenu(this, point);
		}
		break;

	case WM_LBUTTONUP:
		ShowWindow(SW_SHOW);
		SetForegroundWindow();
		break;
	}

	return 0;
}

void CProxyServerDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CMenu *menu = mProxyMenu.GetSubMenu(0);

	menu->EnableMenuItem(ID_MENUITEM_START, mServiceRunning);
	menu->EnableMenuItem(ID_MENUITEM_STOP, mServiceRunning == false);

	if (IsWindowVisible())
	{
		menu->ModifyMenu(ID_MENUITEM_VISIBLE, MF_BYCOMMAND, ID_MENUITEM_VISIBLE, "隐藏主界面");
	}
	else
	{
		menu->ModifyMenu(ID_MENUITEM_VISIBLE, MF_BYCOMMAND, ID_MENUITEM_VISIBLE, "显示主界面");
	}

	menu->TrackPopupMenu(TPM_LEFTALIGN, point.x , point.y, this);
}

void CProxyServerDlg::OnMenuitemExit()
{
	PostQuitMessage(0);
}

void CProxyServerDlg::OnMenuitemVisible()
{
	if (IsWindowVisible())
	{
		ShowWindow(SW_HIDE);
	}
	else
	{
		ShowWindow(SW_SHOW);
	}
}
