// emmc_buildDlg.cpp : implementation file
//

#include "stdafx.h"
#include "emmc_build.h"
#include "emmc_buildDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define KB(a)			(a << 10)
#define MB(a)			(a << 20)
#define GB(a)			(a << 30)

#define MAX_BUFF_LEN	(1024)

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
// CEmmc_buildDlg dialog

CEmmc_buildDlg::CEmmc_buildDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEmmc_buildDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEmmc_buildDlg)
	m_edit_busybox = _T("");
	m_edit_uboot = _T("");
	m_edit_uimage = _T("");
	m_static_status = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEmmc_buildDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEmmc_buildDlg)
	DDX_Control(pDX, IDC_PROGRESS_BUILD, m_progress_build);
	DDX_Text(pDX, IDC_EDIT_BUSYBOX, m_edit_busybox);
	DDX_Text(pDX, IDC_EDIT_UBOOT, m_edit_uboot);
	DDX_Text(pDX, IDC_EDIT_UIMAGE, m_edit_uimage);
	DDX_Text(pDX, IDC_STATIC_STATUS, m_static_status);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CEmmc_buildDlg, CDialog)
	//{{AFX_MSG_MAP(CEmmc_buildDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_UBOOT, OnButtonUboot)
	ON_BN_CLICKED(IDC_BUTTON_UIMAGE, OnButtonUimage)
	ON_BN_CLICKED(IDC_BUTTON_BUSYBOX, OnButtonBusybox)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmmc_buildDlg message handlers

BOOL CEmmc_buildDlg::OnInitDialog()
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

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CEmmc_buildDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CEmmc_buildDlg::OnPaint()
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
HCURSOR CEmmc_buildDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CEmmc_buildDlg::OnOK()
{
	// TODO: Add extra validation here
	CFileDialog fileDlg(false, "bin", "factory", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "factory文件|factory*.bin||");

	fileDlg.m_ofn.lpstrTitle = "创建工厂专用factory.bin";

	if (fileDlg.DoModal() != IDOK)
	{
		return;
	}

	if (BuileFactoryImage(fileDlg.GetPathName()))
	{
		MessageBox("恭喜您，创建成功", "正确", MB_ICONINFORMATION);
		m_static_status = "创建成功";
	}
	else
	{
		MessageBox("创建失败!!!", "错误", MB_ICONERROR);
		m_static_status = "创建失败";
	}

	UpdateData(false);

	// CDialog::OnOK();
}

void CEmmc_buildDlg::OnButtonUboot()
{
	// TODO: Add your control notification handler code here
	CFileDialog fileDlg(true, "bin", "u-boot-factory", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST, "uboot文件|u-boot*.bin||");

	if (fileDlg.DoModal() == IDOK)
	{
		m_edit_uboot = fileDlg.GetPathName();
		UpdateData(false);
	}
}

void CEmmc_buildDlg::OnButtonUimage()
{
	// TODO: Add your control notification handler code here
	CFileDialog fileDlg(true, NULL, "uImage", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST, "uImage文件|uImage*||");

	if (fileDlg.DoModal() == IDOK)
	{
		m_edit_uimage = fileDlg.GetPathName();
		UpdateData(false);
	}
}

void CEmmc_buildDlg::OnButtonBusybox()
{
	// TODO: Add your control notification handler code here
	CFileDialog fileDlg(true, "img", "busybox", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST, "busybox文件|busybox*.img||");

	if (fileDlg.DoModal() == IDOK)
	{
		m_edit_busybox = fileDlg.GetPathName();
		UpdateData(false);
	}
}

int CEmmc_buildDlg::BuileFactoryImage(CString &csFileName)
{
	CFile fileImage;
	BOOL bRet;

	bRet = fileImage.Open(csFileName, CFile::modeWrite | CFile::modeCreate, NULL);
	if (bRet == false)
	{
		m_static_status.Format("打开文件\"%s\"失败", csFileName);
		goto out_update_data;
	}

	UpdateData(true);

	bRet = AppendFileToImage(m_edit_uboot, fileImage, 0);
	if (bRet == false)
	{
		goto out_close_file;
	}

	bRet = AppendFileToImage(m_edit_uimage, fileImage, MB(1));
	if (bRet == false)
	{
		goto out_close_file;
	}

	bRet = AppendFileToImage(m_edit_busybox, fileImage, MB(5));
	if (bRet == false)
	{
		goto out_close_file;
	}

out_close_file:
	fileImage.Close();
out_update_data:
	UpdateData(false);
	return bRet;
}

int CEmmc_buildDlg::AppendFileToImage(LPCTSTR lpszFileName, CFile &fileImage, LONG lOffset)
{
	CFile file;
	BOOL bRet;

	bRet = file.Open(lpszFileName, CFile::modeRead, NULL);
	if (bRet == false)
	{
		m_static_status.Format("打开文件\"%s\"失败", lpszFileName);
		goto out_update_data;
	}

	try
	{
		if (fileImage.Seek(lOffset, CFile::begin) != lOffset)
		{
			m_static_status = "移动文件指针失败";
			bRet = false;
			goto out_close_file;
		}
	}
	catch (...)
	{
		m_static_status = "移动文件指针出错";
		UpdateData(false);
		bRet = false;
		goto out_close_file;
	}

	char buff[MAX_BUFF_LEN];
	UINT nReadLen, nTotalLen;

	m_progress_build.SetRange32(0, file.GetLength());
	m_progress_build.SetPos(0);

	m_static_status.Format("正在写入\"%s\"", lpszFileName);
	UpdateData(false);

	do {
		try
		{
			nReadLen = file.Read(buff, sizeof(buff));
			fileImage.Write(buff,  sizeof(buff));
			m_progress_build.OffsetPos(nReadLen);
		}
		catch (...)
		{
			m_static_status = "文件读写出错";
			bRet = false;
			goto out_close_file;
		}
		nTotalLen -= nReadLen;
	} while (nReadLen == sizeof(buff));

	bRet = true;
out_close_file:
	file.Close();
out_update_data:
	UpdateData(false);
	return bRet;
}
