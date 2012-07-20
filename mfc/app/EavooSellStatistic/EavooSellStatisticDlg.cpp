// EavooSellStatisticDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EavooSellStatistic.h"
#include "EavooSellStatisticDlg.h"
#include "EavooSellStatisticDlg2.h"

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
// CEavooSellStatisticDlg dialog

CEavooSellStatisticDlg::CEavooSellStatisticDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEavooSellStatisticDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEavooSellStatisticDlg)
	m_edit_port = 0;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	mSocket = -1;
	mIsOpened = false;
}

void CEavooSellStatisticDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEavooSellStatisticDlg)
	DDX_Control(pDX, IDC_STATIC_state, m_static_state);
	DDX_Control(pDX, IDC_IPADDRESS1, m_ipaddress1);
	DDX_Control(pDX, IDC_LIST_sms, m_list_sms);
	DDX_Text(pDX, IDC_EDIT_port, m_edit_port);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CEavooSellStatisticDlg, CDialog)
	//{{AFX_MSG_MAP(CEavooSellStatisticDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_connect, OnBUTTONconnect)
	ON_BN_CLICKED(IDC_BUTTON_disconnect, OnBUTTONdisconnect)
	ON_BN_CLICKED(IDC_BUTTON_statistic, OnBUTTONstatistic)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEavooSellStatisticDlg message handlers

BOOL CEavooSellStatisticDlg::OnInitDialog()
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
	m_list_sms.InsertColumn(0, "手机号码", LVCFMT_LEFT, 150);
	m_list_sms.InsertColumn(1, "发送时间", LVCFMT_LEFT, 160);
	m_list_sms.InsertColumn(2, "内容", LVCFMT_LEFT, 260);

	m_edit_port = DEFAULT_SERVER_PORT;
	m_ipaddress1.SetWindowText(DEFAULT_SERVER_IP);
	m_static_state.SetWindowText("停止");

	UpdateData(false);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CEavooSellStatisticDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CEavooSellStatisticDlg::OnPaint()
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
HCURSOR CEavooSellStatisticDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CEavooSellStatisticDlg::CloseSocket(void)
{
	if (mSocket != -1)
	{
		closesocket(mSocket);
		mSocket = -1;
	}
}

// EDUA# + 手机的IMEI号 + “,” + 手机软件版本号 + “,,,,#”

int CEavooSellStatisticDlg::ThreadHandler(void *data)
{
	int recvLen;
	char buff[4096], *p, *p_end;
	CEavooSellStatisticDlg *dlg = (CEavooSellStatisticDlg *)data;
	int sockfd = dlg->mSocket;

	dlg->mIsOpened = dlg->mFile.Open(CACHE_FILENAME, CFile::modeWrite | CFile::modeCreate | CFile::modeNoTruncate | CFile::shareDenyNone, NULL);
	if (dlg->mIsOpened == false)
	{
		AfxMessageBox("打开文件失败");
	}

	dlg->mFile.SeekToEnd();

	AfxMessageBox("服务器已启动");
	dlg->m_static_state.SetWindowText("正在运行...");

	p = buff;
	p_end = buff + sizeof(buff);
	dlg->m_list_sms.DeleteAllItems();

	while (1)
	{
		recvLen = recv(sockfd, p, p_end - p, 0);
		if (recvLen == SOCKET_ERROR || recvLen == 0)
		{
			break;
		}

		p = dlg->AdbParseDataMain(buff, p + recvLen);
	}

	if (dlg->mIsOpened)
	{
		dlg->mFile.Close();
		dlg->mIsOpened = false;
	}

	dlg->CloseSocket();

	AfxMessageBox("服务器已停止工作");
	dlg->m_static_state.SetWindowText("服务器已停止工作");

	return 0;
}

int CEavooSellStatisticDlg::AdbConnect(void)
{
	int ret;
	char ip_address[32];

	m_ipaddress1.GetWindowText(ip_address, sizeof(ip_address));

	sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = inet_addr(ip_address);
	addr.sin_port = htons(ADB_SERVICE_PORT);

	ret = connect(mSocket, (sockaddr *)&addr, sizeof(addr));
	if (ret == 0)
	{
		return 0;
	}

	ret = system("adb start-server");
	if (ret != 0)
	{
		AfxMessageBox("请安装ADB命令");
		return -1;
	}

	return connect(mSocket, (sockaddr *)&addr, sizeof(addr));
}

void CEavooSellStatisticDlg::OnBUTTONconnect()
{
	// TODO: Add your control notification handler code here
	if (mSocket != -1)
	{
		AfxMessageBox("已经建立了连接，请先断开连接");
		return;
	}

	UpdateData(true);

	int ret = WSAStartup(MAKEWORD(2, 2), &mWsaData);
	if (ret != 0)
	{
		AfxMessageBox("启动协议失败");
		return;
	}

	mSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (mSocket == INVALID_SOCKET)
	{
		AfxMessageBox("分配套接字失败");
		return;
	}

	ret = AdbConnect();
	if (ret != 0)
	{
		AfxMessageBox("建立连接失败，请确认IP地址是否正确");
		CloseSocket();
		return;
	}

	char command[16];
	sprintf(command, "tcp:%04d", m_edit_port);
	ret = AdbSendCommand(command);
	if (ret < 0)
	{
		AfxMessageBox("建立连接失败，请确认端口号是否正确");
		CloseSocket();
		return;
	}

	AfxBeginThread((AFX_THREADPROC)CEavooSellStatisticDlg::ThreadHandler, this);
}

void CEavooSellStatisticDlg::OnBUTTONdisconnect()
{
	// TODO: Add your control notification handler code here
	CloseSocket();
}

int CEavooSellStatisticDlg::TextCmpLH(const char *left, const char *right)
{
	while (*left)
	{
		if (*left != *right)
		{
			return *left - *right;
		}

		left++;
		right++;
	}

	return 0;
}

int CEavooSellStatisticDlg::AdbReadStatus(void)
{
	int recvLen;

	recvLen = recv(mSocket, mAdbStatus, 4, 0);
	if (recvLen != 4)
	{
		sprintf(mAdbStatus, "接收状态信息失败");
		return -1;
	}

	if (TextCmpLH(ADB_STATE_OKAY, mAdbStatus) == 0)
	{
		return 0;
	}

	if (TextCmpLH(ADB_STATE_FAIL, mAdbStatus) == 0)
	{
		return -1;
	}

	int length;

	sscanf(mAdbStatus, "%04x", &length);
	if (length > sizeof(mAdbStatus))
	{
		length = sizeof(mAdbStatus);
	}

	recvLen = recv(mSocket, mAdbStatus, length, 0);
	if (recvLen != length)
	{
		sprintf(mAdbStatus, "接收状态信息失败");
		return -1;
	}

	return -1;
}

int CEavooSellStatisticDlg::AdbSendText(const char *text)
{
	int sendLen;
	int length = strlen(text);
	char buff[32];

	sprintf(buff, "%04x", length);
	sendLen = send(mSocket, buff, 4, 0);
	if (sendLen != 4)
	{
		AfxMessageBox("发送命令长度失败");
		return sendLen;
	}

	sendLen = send(mSocket, text, length, 0);
	if (sendLen != length)
	{
		AfxMessageBox("发送命令长度失败");
		return sendLen;
	}

	return AdbReadStatus();
}

int CEavooSellStatisticDlg::AdbSendCommand(const char *command)
{
	int ret;

	if (TextCmpLH("host", command))
	{
		ret = AdbSendText("host:transport-any");
		if (ret < 0)
		{
			return ret;
		}
	}

	return AdbSendText(command);
}

char *CEavooSellStatisticDlg::TextFindLineEnd(const char *buff, const char *end)
{
	while (buff < end)
	{
		if (*buff == '\r' || *buff == '\n')
		{
			return (char *)buff;
		}

		buff++;
	}

	return NULL;
}

char *CEavooSellStatisticDlg::AdbParseDataSingle(const char *buff, const char *end, char *segments[], int size)
{
	int i;
	char *p;
	end = TextFindLineEnd(buff, end);
	if (end == NULL)
	{
		return NULL;
	}

	for (i = 0, size--; i < size; i++)
	{
		for (p = segments[i]; buff < end; p++, buff++)
		{
			if (*buff == ',')
			{
				break;
			}

			*p = *buff;
		}

		buff++;
		*p = 0;
	}

	for (p = segments[i]; buff < end; buff++, p++)
	{
		*p = *buff;
	}

	*p = 0;

	return (char *)end;
}

char *CEavooSellStatisticDlg::AdbParseDataMulti(const char *buff, const char *end)
{
	char mobile[32];
	char time[32];
	char content[1024];
	char *segments[] =
	{
		mobile, time, content
	};

	while (1)
	{
 		const char *temp = AdbParseDataSingle(buff, end, segments, NELEM(segments));
		if (temp == NULL)
		{
			break;
		}

		m_list_sms.InsertItem(0, mobile);
		m_list_sms.SetItemText(0, 1, time);
		m_list_sms.SetItemText(0, 2, content);

		for (buff = temp; buff < end && (*buff == '\r' || *buff == '\n'); buff++);
	}

	return (char *)buff;
}

char *CEavooSellStatisticDlg::AdbParseDataMain(char *buff, char *end)
{
	char *p = AdbParseDataMulti(buff, end);

	if (mIsOpened)
	{
		mFile.Write(buff, p - buff);
	}

	while (p < end)
	{
		*buff++ = *p++;
	}

	return buff;
}

void CEavooSellStatisticDlg::OnBUTTONstatistic()
{
	CEavooSellStatisticDlg2 Dlg;

	Dlg.DoModal();
}
