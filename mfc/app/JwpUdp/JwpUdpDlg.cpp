// JwpUdpDlg.cpp : implementation file
//

#include "stdafx.h"
#include "JwpUdp.h"
#include "JwpUdpDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
// CJwpUdpDlg dialog

CJwpUdpDlg::CJwpUdpDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CJwpUdpDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CJwpUdpDlg)
	m_LogValue = _T("");
	m_PortValue = 0;
	m_DataValue = _T("");
	m_StateValue = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CJwpUdpDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CJwpUdpDlg)
	DDX_Control(pDX, IDC_IPADDRESS, m_IpAddressCtrl);
	DDX_Text(pDX, IDC_EDIT_LOG, m_LogValue);
	DDX_Text(pDX, IDC_EDIT_PORT, m_PortValue);
	DDX_Text(pDX, IDC_EDIT_DATA, m_DataValue);
	DDX_Text(pDX, IDC_STATIC_STATE, m_StateValue);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CJwpUdpDlg, CDialog)
	//{{AFX_MSG_MAP(CJwpUdpDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START, OnButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_SEND_COMMAND, OnButtonSendCommand)
	ON_BN_CLICKED(IDC_BUTTON_SEND_DATA, OnButtonSendData)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, OnButtonClear)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJwpUdpDlg message handlers

BOOL CJwpUdpDlg::OnInitDialog()
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
	m_IpAddressCtrl.SetAddress(192, 168, 1, 19);
	m_PortValue = 1234;
	UpdateData(false);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CJwpUdpDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CJwpUdpDlg::OnPaint() 
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
HCURSOR CJwpUdpDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CJwpUdpDlg::OnSendComplete(void)
{
	println("OnSendComplete");
}

void CJwpUdpDlg::OnDataReceived(const void *data, jwp_size_t size)
{
	char buff[1024];

	memcpy(buff, buff, size);
	buff[size] = 0;

	println("OnDataReceived: data = %s, size = %d", buff, size);
}

void CJwpUdpDlg::OnCommandReceived(const void *command, jwp_size_t size)
{
	char buff[1024];

	memcpy(buff, command, size);
	buff[size] = 0;

	println("OnCommandReceived: command = %s, size = %d", buff, size);
}

void CJwpUdpDlg::OnPackageReceived(const struct jwp_header *hdr)
{
	println("OnPackageReceived: index = %d, type = %d, length = %d", hdr->index, hdr->type, hdr->length);
}

void CJwpUdpDlg::OnButtonStart() 
{
	DWORD addr;

	UpdateData(true);
	m_IpAddressCtrl.GetAddress(addr);

	if (StartJwp(addr, m_PortValue))
	{
		m_StateValue = "已连接";
	}
	else
	{

		m_StateValue = "连接失败";
	}

	UpdateData(false);
}

void CJwpUdpDlg::OnButtonStop() 
{
	StopJwp();
	m_StateValue = "连接断开";
}

void CJwpUdpDlg::OnButtonSendCommand() 
{
	UpdateData(true);
	SendCommand(m_DataValue, m_DataValue.GetLength());
}

void CJwpUdpDlg::OnButtonSendData() 
{
	UpdateData(true);
	SendData(m_DataValue, m_DataValue.GetLength());
}

void CJwpUdpDlg::OnButtonClear() 
{
	m_LogValue.Empty();
	UpdateData(false);
}
