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
	mThread = NULL;
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

CEavooShortMessage::CEavooShortMessage(CFile &file, SOCKET sockfd, CListCtrl *list) : mFile(file)
{
	mSocket = sockfd;
	mListCtrl = list;
}

CEavooShortMessage::~CEavooShortMessage()
{
	Uninitialize();
}

int CEavooShortMessage::Initialize(void)
{
	if (mListCtrl)
	{
		mListCtrl->DeleteAllItems();
	}

	return 0;
}

int CEavooShortMessage::Uninitialize(void)
{
	mFile.Close();

	if (socket > 0)
	{
		closesocket(mSocket);
	}

	return 0;
}

int CEavooShortMessage::ReceiveData(char *buff, int size)
{
	return recv(mSocket, buff, size, 0) == size ? size : -1;
}

int CEavooShortMessage::SendData(const char *buff, int size)
{
	return send(mSocket, buff, size, 0) == size ? size : -1;
}

int CEavooShortMessage::ReceiveText(char *buff)
{
	int length;

	if (ReceiveData((char *)&length, sizeof(length)) < 0 || ReceiveData(buff, length) < 0)
	{
		return -1;
	}

	buff[length] = 0;

	return length;
}

int CEavooShortMessage::SendResponse(char type)
{
	return SendData(&type, 1);
}

int CEavooShortMessage::ReceiveFromNetwork(void)
{
	char type;
	int ret;

	while (1)
	{
		if (ReceiveData(&type, sizeof(type)) < 0)
		{
			return -1;
		}

		switch (type)
		{
		case SMS_TYPE_DATE:
			if (ReceiveData((char *)&mDate, sizeof(mDate)) < 0)
			{
				return -1;
			}

			if (WriteData(type, (char *)&mDate, sizeof(mDate)) < 0)
			{
				return -1;
			}
			break;

		case SMS_TYPE_PHONE:
			ret = ReceiveText(mPhone);
			if (ret < 0)
			{
				return ret;
			}

			if (WriteText(type, mPhone, ret) < 0)
			{
				return -1;
			}
			break;

		case SMS_TYPE_CONTENT:
			ret = ReceiveText(mContent);
			if (ret < 0)
			{
				return ret;
			}

			if (WriteText(type, mContent, ret) < 0)
			{
				return -1;
			}
			break;

		case SMS_TYPE_TEST:
			if (SendResponse(SMS_TYPE_ACK) < 0)
			{
				return -1;
			}
			continue;

		case SMS_TYPE_END:
			if (SendResponse(SMS_TYPE_ACK) < 0)
			{
				return -1;
			}

			if (WriteType(type) < 0)
			{
				return -1;
			}
			return 0;

		default:
			return -1;
		}
	}

	return -1;
}

void CEavooShortMessage::InsertIntoList(void)
{
	mListCtrl->InsertItem(0, mPhone);
	mListCtrl->SetItemText(0, 1, ctime(&mDate));
	mListCtrl->SetItemText(0, 2, mContent);
}

int CEavooShortMessage::WriteType(char type)
{
	mFile.Write(&type, 1);

	return 1;
}

int CEavooShortMessage::WriteText(char type, const char *text)
{
	mFile.Write(&type, 1);

	int length = strlen(text);
	mFile.Write(&length, sizeof(length));
	mFile.Write(text, length);

	return length;
}

int CEavooShortMessage::WriteText(char type, const char *text, int length)
{
	mFile.Write(&type, 1);
	mFile.Write(&length, sizeof(length));
	mFile.Write(text, length);

	return length;
}

int CEavooShortMessage::WriteData(char type, const char *data, int size)
{
	mFile.Write(&type, 1);
	mFile.Write(data, size);

	return size;
}

int CEavooShortMessage::WriteToFile(void)
{
	WriteData(SMS_TYPE_DATE, (char *)&mDate, sizeof(mDate));
	WriteText(SMS_TYPE_PHONE, mPhone);
	WriteText(SMS_TYPE_CONTENT, mContent);
	WriteType(SMS_TYPE_END);

	return 0;
}

int CEavooShortMessage::ReadData(char *buff, UINT size)
{
	return mFile.Read(buff, size) == size ? size : -1;
}

int CEavooShortMessage::ReadText(char *buff)
{
	int length;

	if (ReadData((char *)&length, sizeof(length)) < 0)
	{
		return -1;
	}

	if (ReadData(buff, length) < 0)
	{
		return -1;
	}

	buff[length] = 0;

	return length;
}

int CEavooShortMessage::ReadFromFile(void)
{
	char type;

	while (1)
	{
		if (ReadData(&type, 1) < 0)
		{
			return -1;
		}

		switch (type)
		{
		case SMS_TYPE_DATE:
			if (ReadData((char *)&mDate, sizeof(mDate)) < 0)
			{
				return -1;
			}
			break;

		case SMS_TYPE_PHONE:
			if (ReadText(mPhone) < 0)
			{
				return -1;
			}
			break;

		case SMS_TYPE_CONTENT:
			if (ReadText(mContent) < 0)
			{
				return -1;
			}

		case SMS_TYPE_END:
			return 0;

		default:
			return -1;
		}
	}

	return 0;
}

// EDUA# + 手机的IMEI号 + “,” + 手机软件版本号 + “,,,,#”

int CEavooSellStatisticDlg::ThreadHandler(void *data)
{
	CEavooSellStatisticDlg *dlg = (CEavooSellStatisticDlg *)data;
	CFile file;

	if (file.Open(CACHE_FILENAME, CFile::modeWrite | CFile::shareDenyNone | CFile::modeCreate | CFile::modeNoTruncate, NULL) == false)
	{
		return -1;
	}

	file.SeekToEnd();

	if (dlg->AdbConnectServer() < 0)
	{
		dlg->mThread = NULL;
		return -1;
	}

	CEavooShortMessage message(file, dlg->mSocket, &dlg->m_list_sms);
	if (message.Initialize() < 0)
	{
		dlg->mThread = NULL;
		return -1;
	}

	AfxMessageBox("服务器已启动");
	dlg->m_static_state.SetWindowText("正在运行...");

	while (1)
	{
		if (message.ReceiveFromNetwork() < 0)
		{
			break;
		}

		if (message.WriteToFile() < 0)
		{
			break;
		}

		message.InsertIntoList();
	}

	dlg->mThread = NULL;
	AfxMessageBox("服务器已停止工作");
	dlg->m_static_state.SetWindowText("服务器已停止工作");

	return 0;
}

int CEavooSellStatisticDlg::AdbConnectServer(void)
{
	if (WSAStartup(MAKEWORD(2, 2), &mWsaData) != 0)
	{
		AfxMessageBox("启动协议失败");
		return -1;
	}

	mSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (mSocket == INVALID_SOCKET)
	{
		AfxMessageBox("分配套接字失败");
		return -1;
	}

	if (AdbConnectTcp() != 0)
	{
		AfxMessageBox("建立连接失败，请确认IP地址是否正确");
		closesocket(mSocket);
		return -1;
	}

	char command[16];
	sprintf(command, "tcp:%04d", m_edit_port);
	if (AdbSendCommand(command) < 0)
	{
		AfxMessageBox("建立连接失败，请确认端口号是否正确");
		closesocket(mSocket);
		return -1;
	}

	return 0;
}

int CEavooSellStatisticDlg::AdbConnectTcp(void)
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
	if (mThread == NULL)
	{
		mThread = AfxBeginThread((AFX_THREADPROC)CEavooSellStatisticDlg::ThreadHandler, this);
	}
}

void CEavooSellStatisticDlg::OnBUTTONdisconnect()
{
	// TODO: Add your control notification handler code here
	closesocket(mSocket);
}

int CEavooSellStatisticDlg::ReceiveData(char *buff, int size)
{
	return recv(mSocket, buff, size, 0) == size ? size : -1;
}

int CEavooSellStatisticDlg::AdbReadStatus(void)
{
	if (ReceiveData(mAdbStatus, 4) < 0)
	{
		sprintf(mAdbStatus, "接收状态信息失败");
		return -1;
	}

	if (strncmp(ADB_STATE_OKAY, mAdbStatus, 4) == 0)
	{
		return 0;
	}

	if (strncmp(ADB_STATE_FAIL, mAdbStatus, 4) == 0)
	{
		return -1;
	}

	int length;

	sscanf(mAdbStatus, "%04x", &length);
	if (length > sizeof(mAdbStatus))
	{
		length = sizeof(mAdbStatus);
	}

	if (ReceiveData(mAdbStatus, length) < 0)
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

	if (strncmp("host", command, 4))
	{
		ret = AdbSendText("host:transport-any");
		if (ret < 0)
		{
			return ret;
		}
	}

	return AdbSendText(command);
}

void CEavooSellStatisticDlg::OnBUTTONstatistic()
{
	CEavooSellStatisticDlg2 Dlg;

	Dlg.DoModal();
}
