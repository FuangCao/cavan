// swan_upgradeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "swan_upgrade.h"
#include "swan_upgradeDlg.h"
#include <shlobj.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IMAGE_TYPE_INDEX		1
#define IMAGE_MAJOR_INDEX		2
#define IMAGE_MINOR_INDEX		3
#define IMAGE_DEVICE_INDEX		4
#define IMAGE_OFFSET_INDEX		5
#define IMAGE_PATH_INDEX		6

#define MAX_BUFF_LEN			(512 * 100)

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
// CSwan_upgradeDlg dialog

CSwan_upgradeDlg::CSwan_upgradeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSwan_upgradeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSwan_upgradeDlg)
	m_edit_header = _T("");
	m_static_status = _T("");
	m_edit_version = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSwan_upgradeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSwan_upgradeDlg)
	DDX_Control(pDX, IDC_PROGRESS_BUILD, m_progress_build);
	DDX_Control(pDX, IDC_LIST_IMAGES, m_list_images);
	DDX_Text(pDX, IDC_EDIT_HEADER, m_edit_header);
	DDX_Text(pDX, IDC_STATIC_STATUS, m_static_status);
	DDX_Text(pDX, IDC_EDIT_VERSION, m_edit_version);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSwan_upgradeDlg, CDialog)
	//{{AFX_MSG_MAP(CSwan_upgradeDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_EDIT, OnButtonEdit)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, OnButtonDelete)
	ON_BN_CLICKED(IDC_BUTTON_HEADER, OnButtonHeader)
	ON_BN_CLICKED(IDC_BUTTON_LOAD, OnButtonLoad)
	ON_BN_CLICKED(IDC_BUTTON_DEL_ALL, OnButtonDelAll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSwan_upgradeDlg message handlers

BOOL CSwan_upgradeDlg::OnInitDialog()
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
	m_list_images.SetExtendedStyle(LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);
	m_list_images.InsertColumn(0, "序号", LVCFMT_LEFT, 40);
	m_list_images.InsertColumn(IMAGE_TYPE_INDEX, "文件类型", LVCFMT_LEFT, 140);
	m_list_images.InsertColumn(IMAGE_MAJOR_INDEX, "主设备号", LVCFMT_LEFT, 60);
	m_list_images.InsertColumn(IMAGE_MINOR_INDEX, "次设备号", LVCFMT_LEFT, 60);
	m_list_images.InsertColumn(IMAGE_DEVICE_INDEX, "设备节点", LVCFMT_LEFT, 100);
	m_list_images.InsertColumn(IMAGE_OFFSET_INDEX, "偏移量", LVCFMT_LEFT, 60);
	m_list_images.InsertColumn(IMAGE_PATH_INDEX, "文件路径", LVCFMT_LEFT, 200);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSwan_upgradeDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSwan_upgradeDlg::OnPaint()
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
HCURSOR CSwan_upgradeDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

int CSwan_upgradeDlg::WriteToPackage(CFile &filePkg, LPCTSTR lpszFileName)
{
	CFile fileLocal;
	BOOL bRet;

	bRet = fileLocal.Open(lpszFileName, CFile::modeRead, NULL);
	if (bRet == false)
	{
		m_static_status.Format("打开文件\"%s\"失败", lpszFileName);
		return false;
	}

	bRet = WriteToPackage(filePkg, fileLocal);
	if (bRet == false)
	{
		m_static_status.Format("打开文件\"%s\"失败", lpszFileName);
	}

	fileLocal.Close();

	return bRet;
}

int CSwan_upgradeDlg::WriteToPackage(CFile &filePkg, CFile &fileSrc)
{
	char buff[MAX_BUFF_LEN];
	UINT nReadLen;

	m_progress_build.SetRange32(0, fileSrc.GetLength());
	m_progress_build.SetPos(0);

	m_static_status.Format("正在写入\"%s\"", fileSrc.GetFilePath());
	UpdateData(false);

	do
	{
		try
		{
			nReadLen = fileSrc.Read(buff, sizeof(buff));
			filePkg.Write(buff, nReadLen);
			m_progress_build.OffsetPos(nReadLen);
		}
		catch (...)
		{
			return false;
		}
	} while (nReadLen == sizeof(buff));

	return true;
}

int CSwan_upgradeDlg::WriteToPackage(CFile &filePkg, const void *lpBuff, UINT nCount)
{
	try
	{
		filePkg.Write(lpBuff, nCount);
	}
	catch (...)
	{
		return false;
	}

	return true;
}

int CSwan_upgradeDlg::WriteToPackage(CFile &filePkg, CSwanImage *pImage, LPCTSTR lpszImagePath)
{
	BOOL bRet;

	bRet = WriteToPackage(filePkg, pImage, sizeof(*pImage));
	if (bRet == false)
	{
		m_static_status = "写Image头出错";
		return false;
	}

	bRet = WriteToPackage(filePkg, lpszImagePath);
	if (bRet == false)
	{
		return false;
	}

	return true;
}

int CSwan_upgradeDlg::WriteHeader(CFile &filePkg, CFileInfo &fileInfo, LPCTSTR lpszHeaderPath)
{
	CFile fileHeader;
	BOOL bRet;

	bRet = fileHeader.Open(lpszHeaderPath, 0, NULL);
	if (bRet == false)
	{
		m_static_status.Format("打开文件\"%s\"失败", lpszHeaderPath);
		return false;
	}

	strncpy(fileInfo.cCheckPattern, CHECK_PATTERN, sizeof(CHECK_PATTERN));
	strncpy(fileInfo.cMd5Sum, "123456", MAX_MD5SUM_LEN);
	fileInfo.nBuildTime = CTime::GetCurrentTime().GetTime();
	fileInfo.dwOffset = fileHeader.GetLength() + sizeof(CFileInfo);

	bRet = WriteToPackage(filePkg, &fileInfo, sizeof(fileInfo));
	if (bRet == false)
	{
		m_static_status = "写文件头失败";
		goto out_close_header;
	}

	bRet = WriteToPackage(filePkg, fileHeader);
	if (bRet == false)
	{
		m_static_status.Format("读写文件\"%s\"失败", lpszHeaderPath);
	}

out_close_header:
	fileHeader.Close();

	return bRet;
}

void CSwan_upgradeDlg::OnOK()
{
	// TODO: Add extra validation here
	UpdateData(true);

	if (m_edit_header.IsEmpty())
	{
		MessageBox("请选择Header.bin文件的路径");
		return;
	}

	if (m_edit_version.IsEmpty())
	{
		MessageBox("请输入升级文件的版本号");
		return;
	}

	if (m_list_images.GetItemCount() == 0)
	{
		MessageBox("请添加Image");
		return;
	}

	CFileDialog fileDlg(false, "swan", "upgrade_eavoo",  OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "upgrade文件|upgrade*.swan||");

	fileDlg.m_ofn.lpstrTitle = "生成升级文件";

	if (fileDlg.DoModal() != IDOK)
	{
		return;
	}

	CFile filePkg;
	BOOL bRet;

	bRet = filePkg.Open(fileDlg.GetPathName(), CFile::modeCreate | CFile::modeWrite, NULL);
	if (bRet == false)
	{
		m_static_status.Format("打开文件\"%s\"失败", fileDlg.GetPathName());
		goto out_update_data;
	}

	CFileInfo fileInfo;

	bRet = WriteHeader(filePkg, fileInfo, m_edit_header);
	if (bRet == false)
	{
		goto out_close_pkg;
	}

	CPackageInfo pkgInfo;

	pkgInfo.image_count = m_list_images.GetItemCount();
	strncpy(pkgInfo.cVersion, m_edit_version, MAX_VERSION_LEN);

	bRet = WriteToPackage(filePkg, &pkgInfo, sizeof(pkgInfo));
	if (bRet == false)
	{
		goto out_close_pkg;
	}

	CSwanImage *pImage;
	int iIndex;
	int i;

	for (i = 0; i < (int)pkgInfo.image_count; i++)
	{
		iIndex = CEdit_imageDlg::TextToValue(m_list_images.GetItemText(i, 0));
		pImage = CSwan_upgradeApp::GetImageInfo(iIndex);
		if (pImage == NULL)
		{
			bRet = false;
			break;
		}

		bRet = WriteToPackage(filePkg, pImage, m_list_images.GetItemText(i, IMAGE_PATH_INDEX));
		if (bRet == false)
		{
			break;
		}
	}

out_close_pkg:
	filePkg.Close();
out_update_data:
	UpdateData(false);

	if (bRet)
	{
		MessageBox("成功");
	}
	else
	{
		MessageBox("失败!!!");
	}
	// CDialog::OnOK();
}

void CSwan_upgradeDlg::SetListText(int iIndex, CSwanImage *pCurrentImage, const CString &rImagePath)
{
	int iImgIndex = CSwan_upgradeApp::GetImageIndex(pCurrentImage);

	if (iImgIndex == -1)
	{
		return;
	}

	m_list_images.SetItemText(iIndex, 0, CEdit_imageDlg::ValueToText(iImgIndex));
	m_list_images.SetItemText(iIndex, IMAGE_TYPE_INDEX, pCurrentImage->cFileName);
	m_list_images.SetItemText(iIndex, IMAGE_MAJOR_INDEX, CEdit_imageDlg::ValueToText(pCurrentImage->nMajor));
	m_list_images.SetItemText(iIndex, IMAGE_MINOR_INDEX, CEdit_imageDlg::ValueToText(pCurrentImage->nMinor));
	m_list_images.SetItemText(iIndex, IMAGE_DEVICE_INDEX, pCurrentImage->cDevicePath);
	m_list_images.SetItemText(iIndex, IMAGE_OFFSET_INDEX, CEdit_imageDlg::SizeToText(pCurrentImage->dwOffset));
	m_list_images.SetItemText(iIndex, IMAGE_PATH_INDEX, rImagePath);
}

void CSwan_upgradeDlg::OnButtonAdd()
{
	// TODO: Add your control notification handler code here
	CEdit_imageDlg imgDlg;

	if (imgDlg.DoModal() != IDOK)
	{
		return;
	}

	ListAddItem(imgDlg.pCurrentImage, imgDlg.m_edit_path);
}

void CSwan_upgradeDlg::OnButtonEdit()
{
	// TODO: Add your control notification handler code here
	int iSelectedItem = m_list_images.GetSelectionMark();

	if (iSelectedItem == -1)
	{
		return;
	}

	CSwanImage *pImage = CSwan_upgradeApp::GetImageInfo(m_list_images.GetItemText(iSelectedItem, IMAGE_TYPE_INDEX));

	if (pImage == NULL)
	{
		return;
	}

	CSwanImage swanBackup = *pImage;
	CEdit_imageDlg imgDlg(pImage);

	imgDlg.m_edit_path = m_list_images.GetItemText(iSelectedItem, IMAGE_PATH_INDEX);

	if (imgDlg.DoModal() == IDOK)
	{
		SetListText(iSelectedItem, imgDlg.pCurrentImage, imgDlg.m_edit_path);
	}
	else
	{
		*pImage = swanBackup;
	}
}

void CSwan_upgradeDlg::OnButtonDelete()
{
	// TODO: Add your control notification handler code here
	int iSelectedItem = m_list_images.GetSelectionMark();

	if (iSelectedItem == -1)
	{
		return;
	}

	CSwanImage *pImage = CSwan_upgradeApp::GetImageInfo(CEdit_imageDlg::TextToValue(m_list_images.GetItemText(iSelectedItem, 0)));
	if (pImage == NULL)
	{
		return;
	}

	pImage->dwLength = 0;

	m_list_images.DeleteItem(iSelectedItem);
}

void CSwan_upgradeDlg::OnButtonHeader()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);

	CFileDialog fileDlg(true, "bin", "header", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST, "header文件|header*.bin||");

	if (fileDlg.DoModal() == IDOK)
	{
		m_edit_header = fileDlg.GetPathName();
		UpdateData(false);
	}
}

int CSwan_upgradeDlg::ListAddItem(CSwanImage *pImage, const CString &rImagePath)
{
	int iItem = m_list_images.InsertItem(0, "-");

	if (iItem == -1)
	{
		MessageBox("Error: InsertItem");
		return false;
	}

	SetListText(iItem, pImage, rImagePath);

	return true;
}

void CSwan_upgradeDlg::LoadImages(CString rPath)
{
	CFile file;
	CString rImagePath;

	m_list_images.DeleteAllItems();

	for (CSwanImage *pImage = CSwan_upgradeApp::imgSwanImages + MAX_IMAGE_COUNT - 1; \
		pImage >= CSwan_upgradeApp::imgSwanImages; pImage--)
	{
		rImagePath.Format("%s\\%s", rPath, pImage->cFileName);

		if (file.Open(rImagePath, 0, NULL))
		{
			pImage->dwLength = file.GetLength();
			file.Close();

			if (ListAddItem(pImage, rImagePath) == false)
			{
				break;
			}
		}
		else
		{
			pImage->dwLength = 0;
		}
	}

	rImagePath.Format("%s\\header.bin", rPath);

	if (file.Open(rImagePath, 0, NULL))
	{
		file.Close();
		m_edit_header = rImagePath;
	}
	else
	{
		m_edit_header.Empty();
	}

	m_edit_version = rPath.Right(rPath.GetLength() - rPath.ReverseFind('\\') - 1);

	UpdateData(false);
}

void CSwan_upgradeDlg::OnButtonLoad()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);

	char pszPath[MAX_PATH];

	BROWSEINFO bi;
	ITEMIDLIST *pidl;

	bi.hwndOwner = m_hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = pszPath;
	bi.lpszTitle = "请选择存放Image的目录";
	bi.ulFlags = BIF_STATUSTEXT | BIF_RETURNONLYFSDIRS;// BIF_USENEWUI
	bi.lpfn = NULL;
	bi.lParam = 0;
	bi.iImage = 0;

	pidl = SHBrowseForFolder(&bi);

	if (pidl == NULL)
	{
		return;
	}

	if (SHGetPathFromIDList(pidl, pszPath) == NULL)
	{
		return;
	}

	LoadImages(pszPath);
}

void CSwan_upgradeDlg::OnButtonDelAll()
{
	// TODO: Add your control notification handler code here
	m_list_images.DeleteAllItems();

	for (CSwanImage *pImage = CSwan_upgradeApp::imgSwanImages + MAX_IMAGE_COUNT - 1; \
	pImage >= CSwan_upgradeApp::imgSwanImages; pImage--)
	{
		pImage->dwLength = 0;
	}

}
