// emmc_burnerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "emmc_burner.h"
#include "emmc_burnerDlg.h"
#include <afxdlgs.h>
#include <winioctl.h>
#include <process.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define KB(a)					((a) << 10)
#define MB(a)					((a) << 20)
#define GB(a)					((a) << 30)
#define TB(a)					((a) << 40)

#define MAX_BUFF_LEN			(512 * 100)
#define ARRAY_SIZE(a)			(sizeof(a) / sizeof((a)[0]))
#define MAX_DEVICE_COUNT		32

static const DWORD crc_table[256] = {
	0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
	0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
	0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
	0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
	0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
	0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
	0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
	0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
	0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
	0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
	0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
	0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
	0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
	0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
	0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
	0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
	0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
	0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
	0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
	0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
	0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
	0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
	0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
	0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
	0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
	0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
	0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
	0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
	0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
	0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
	0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
	0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
	0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
	0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
	0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
	0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
	0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
	0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
	0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
	0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
	0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
	0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
	0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
	0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
	0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
	0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
	0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
	0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
	0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
	0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
	0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
	0x2d02ef8dL
};

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
// CEmmc_burnerDlg dialog

CEmmc_burnerDlg::CEmmc_burnerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEmmc_burnerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEmmc_burnerDlg)
	m_edit_file = _T("");
	m_static_status = _T("");
	m_static_crc32 = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEmmc_burnerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEmmc_burnerDlg)
	DDX_Control(pDX, IDC_BUTTON_COMPARE, m_button_compare);
	DDX_Control(pDX, IDC_EDIT_FILE, m_edit_file_ctrl);
	DDX_Control(pDX, IDOK, m_button_program);
	DDX_Control(pDX, IDC_BUTTON_FILE, m_button_file);
	DDX_Control(pDX, IDC_BUTTON_SEL_ALL, m_button_sel_all);
	DDX_Control(pDX, IDC_BUTTON_SEL_INVERT, m_button_sel_invert);
	DDX_Control(pDX, IDC_BUTTON_REFLESH, m_button_reflesh);
	DDX_Control(pDX, IDC_BUTTON_DESEL, m_button_desel);
	DDX_Control(pDX, IDC_LIST_DEVICES, m_listbox_devices);
	DDX_Control(pDX, IDC_PROGRESS_BURN, m_progress_burn);
	DDX_Text(pDX, IDC_EDIT_FILE, m_edit_file);
	DDX_Text(pDX, IDC_STATIC_STATUS, m_static_status);
	DDX_Text(pDX, IDC_STATIC_CRC32, m_static_crc32);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CEmmc_burnerDlg, CDialog)
	//{{AFX_MSG_MAP(CEmmc_burnerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_FILE, OnButtonFile)
	ON_BN_CLICKED(IDC_BUTTON_REFLESH, OnButtonReflesh)
	ON_EN_CHANGE(IDC_EDIT_FILE, OnChangeEditFile)
	ON_BN_CLICKED(IDC_BUTTON_SEL_ALL, OnButtonSelAll)
	ON_BN_CLICKED(IDC_BUTTON_SEL_INVERT, OnButtonSelInvert)
	ON_BN_CLICKED(IDC_BUTTON_DESEL, OnButtonDesel)
	ON_BN_CLICKED(IDC_BUTTON_COMPARE, OnButtonCompare)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmmc_burnerDlg message handlers

BOOL CEmmc_burnerDlg::OnInitDialog()
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
	UpdateDevices();
	UpdateImageStatus();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CEmmc_burnerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CEmmc_burnerDlg::OnPaint()
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

void CEmmc_burnerDlg::SizeToText(ULONGLONG ullSize, CString &strSize)
{
	if (ullSize == 0)
	{
		strSize = "0B";
		return;
	}

	ULONGLONG dwTemp;
	CString strTemp;

	strSize.Empty();

	dwTemp = (ullSize >> 40) & 0x3FF;
	if (dwTemp)
	{
		strTemp.Format("%dT", dwTemp);
		strSize += strTemp;
	}

	dwTemp = (ullSize >> 30) & 0x3FF;
	if (dwTemp)
	{
		strTemp.Format("%dG", dwTemp);
		strSize += strTemp;
	}

	dwTemp = (ullSize >> 20) & 0x3FF;
	if (dwTemp)
	{
		strTemp.Format("%dM", dwTemp);
		strSize += strTemp;
	}

	dwTemp = (ullSize >> 10) & 0x3FF;
	if (dwTemp)
	{
		strTemp.Format("%dK", dwTemp);
		strSize += strTemp;
	}

	dwTemp = ullSize & 0x3FF;
	if (dwTemp)
	{
		strTemp.Format("%d[B]", dwTemp);
		strSize += strTemp;
	}
	else
	{
		strSize += "B";
	}
}

void CEmmc_burnerDlg::SizeToText(double dbSize, CString &strSize)
{
	if (dbSize < KB(1))
	{
		strSize.Format("%Ld(B)", dbSize);
	}
	else if (dbSize < MB(1))
	{
		strSize.Format("%0.2lf(KB)", dbSize / KB(1));
	}
	else if (dbSize < GB(1))
	{
		strSize.Format("%0.2lf(MB)", dbSize / MB(1));
	}
	else if (dbSize < TB((ULONGLONG)1))
	{
		strSize.Format("%0.2lf(GB)", dbSize / GB(1));
	}
	else
	{
		strSize.Format("%0.2lf(TB)", dbSize / TB((ULONGLONG)1));
	}
}

BOOL CEmmc_burnerDlg::OpenPhysicalDrive(int nIndex, CFile &fileDevice, UINT nOpenFlags)
{
	CString strDevicePath;

	strDevicePath.Format("\\\\.\\PhysicalDrive%d", nIndex);

	return fileDevice.Open(strDevicePath, nOpenFlags | CFile::shareDenyNone);
}

void CEmmc_burnerDlg::CloseFiles(CFile files[], int nCount)
{
	for (int i = 0; i < nCount; i++)
	{
		files[i].Close();
	}
}

int CEmmc_burnerDlg::OpenPhysicalDrives(CFile fileDevices[], int &nCount, UINT nOpenFlags)
{
	nCount = 0;

	for (int i = m_listbox_devices.GetCount() - 1; i >= 0; i--)
	{
		if (m_listbox_devices.GetCheck(i) == 0)
		{
			continue;
		}

		CString strItemText;

		m_listbox_devices.GetText(i, strItemText);

		if (OpenPhysicalDrive(strItemText[5] - '0', fileDevices[nCount], nOpenFlags) == false)
		{
			SetDlgItemFormatText(IDC_STATIC_STATUS, "Open device \"%s\" failed", strItemText);
			CloseFiles(fileDevices, nCount);
			return false;
		}

		nCount++;
	}

	return true;
}

int CEmmc_burnerDlg::GetDeviceCapability(int nDevIndex, double &dbSize)
{
	BOOL bRet;
	CFile fileDevice;

	bRet = OpenPhysicalDrive(nDevIndex, fileDevice, 0);
	if (bRet == false)
	{
		return false;
	}

	DISK_GEOMETRY geomery;
	DWORD dwRet;

	bRet = DeviceIoControl((HANDLE)fileDevice.m_hFile, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &geomery, sizeof(geomery), &dwRet, NULL);
	if (bRet == false)
	{
		goto out_close_file;
	}

	dbSize = (double)(geomery.BytesPerSector * geomery.SectorsPerTrack * geomery.TracksPerCylinder * geomery.Cylinders.QuadPart);

out_close_file:
	fileDevice.Close();

	return bRet;
}

int CEmmc_burnerDlg::GetDeviceCapability(int iDevIndex, CString &strSize)
{
	BOOL bRet;
	double dbSize;

	bRet = GetDeviceCapability(iDevIndex, dbSize);
	if (bRet == false)
	{
		return false;
	}

	SizeToText(dbSize, strSize);

	return true;
}

static int UpdateImageStatusFunc(void *data)
{
	CEmmc_burnerDlg *dlg = (CEmmc_burnerDlg *)data;

	dlg->UpdateImageStatus();

	return true;
}

void CEmmc_burnerDlg::UpdateImageStatus(void)
{
	BOOL bSuccess;

	m_edit_file_ctrl.EnableWindow(false);
	m_button_file.EnableWindow(false);
	m_button_compare.EnableWindow(false);
	m_button_program.EnableWindow(false);

	if (m_edit_file.IsEmpty())
	{
		SetDlgItemText(IDC_STATIC_CRC32, "Please select a image");
		bSuccess = false;
	}
	else if (FileCRC32(m_edit_file, dwImageCRC32, 0) == false)
	{
		SetDlgItemText(IDC_STATIC_CRC32, "Calculate image crc32 failed");
		bSuccess = false;
	}
	else
	{
		SetDlgItemFormatText(IDC_STATIC_CRC32, "Image CRC32 = 0x%08x", dwImageCRC32);
		bSuccess = true;
	}

	if (bSuccess)
	{
		m_button_compare.EnableWindow(true);
		m_button_program.EnableWindow(true);
	}

	m_edit_file_ctrl.EnableWindow(true);
	m_button_file.EnableWindow(true);
}

void CEmmc_burnerDlg::UpdateDevices(void)
{
	m_listbox_devices.ResetContent();

#if 0
	TCHAR buff[1024];

	GetLogicalDriveStrings(sizeof(buff) / sizeof(TCHAR), buff);

	for (LPCTSTR sDrivePath = buff; sDrivePath[0]; sDrivePath += _tcslen(sDrivePath) + 1)
	{
		if (GetDriveType(sDrivePath) == DRIVE_REMOVABLE)
		{
			m_combo_device.AddString(sDrivePath);
		}
	}
#else
	CFile file;
	CString strTemp;
	CString strSize;

	for (int i = 1; i <= 26; i++)
	{
		if(GetDeviceCapability(i, strSize))
		{
			strTemp.Format("Disk[%d] Capability is %s", i, strSize);
			m_listbox_devices.AddString(strTemp);
		}
	}
#endif

	if (m_listbox_devices.GetCount())
	{
		m_button_sel_all.EnableWindow(true);
		m_button_sel_invert.EnableWindow(true);
		m_button_desel.EnableWindow(true);
		OnButtonSelAll();
	}
	else
	{
		m_button_sel_all.EnableWindow(false);
		m_button_sel_invert.EnableWindow(false);
		m_button_desel.EnableWindow(false);
	}
}

int CEmmc_burnerDlg::WriteImage(void)
{
	CFile fileDevices[MAX_DEVICE_COUNT], fileImage;
	BOOL bRet;
	int nCount;

	bRet = OpenPhysicalDrives(fileDevices, nCount, CFile::modeWrite);
	if (bRet == false)
	{
		SetDlgItemText(IDC_STATIC_STATUS, "Open device failed");
		goto out_close_device;
	}

	if (nCount == 0)
	{
		SetDlgItemText(IDC_STATIC_STATUS, "Please select devices");
		return true;
	}

	bRet = fileImage.Open(m_edit_file, CFile::modeRead, NULL);
	if (bRet == false)
	{
		SetDlgItemFormatText(IDC_STATIC_STATUS, "Open image \"%s\" failed", m_edit_file);
		goto out_close_device;
	}

	char buff[MAX_BUFF_LEN];
	UINT nReadLen;

	m_progress_burn.SetRange32(0, fileImage.GetLength());
	m_progress_burn.SetPos(0);

	SetDlgItemText(IDC_STATIC_STATUS, "Programming ...");

	try
	{
		while (1)
		{
			nReadLen = fileImage.Read(buff, sizeof(buff));
			if (nReadLen == 0)
			{
				break;
			}

			for (int i = 0; i < nCount; i++)
			{
				fileDevices[i].Write(buff, sizeof(buff));
			}

			m_progress_burn.OffsetPos(nReadLen);
		}
	}
	catch (...)
	{
		bRet = false;
		SetDlgItemText(IDC_STATIC_STATUS, "Read or Write failed");
		goto out_close_image;
	}

	bRet = true;
	SetDlgItemText(IDC_STATIC_STATUS, "Programming is OK");

out_close_image:
	fileImage.Close();
out_close_device:
	CloseFiles(fileDevices, nCount);

	return bRet;
}

int CEmmc_burnerDlg::CheckDevices(void)
{
	CFile fileDevices[MAX_DEVICE_COUNT], fileImage;
	BOOL bRet;
	int nCount;

	bRet = OpenPhysicalDrives(fileDevices, nCount, CFile::modeRead);
	if (bRet == false)
	{
		SetDlgItemText(IDC_STATIC_STATUS, "Open device failed");
		goto out_close_device;
	}

	if (nCount == 0)
	{
		SetDlgItemText(IDC_STATIC_STATUS, "Please select devices");
		return true;
	}

	bRet = fileImage.Open(m_edit_file, CFile::modeRead, NULL);
	if (bRet == false)
	{
		SetDlgItemFormatText(IDC_STATIC_STATUS, "Open image \"%s\" failed", m_edit_file);
		goto out_close_device;
	}

	char buffImage[MAX_BUFF_LEN];
	char buffDevice[MAX_BUFF_LEN];
	UINT nReadLen;

	m_progress_burn.SetRange32(0, fileImage.GetLength());
	m_progress_burn.SetPos(0);

	SetDlgItemText(IDC_STATIC_STATUS, "Comparison ...");

	try
	{
		while (1)
		{
			nReadLen = fileImage.Read(buffImage, sizeof(buffImage));
			if (nReadLen == 0)
			{
				break;
			}

			for (int i = 0; i < nCount; i++)
			{
				fileDevices[i].Read(buffDevice, sizeof(buffDevice));

				if (memcmp(buffImage, buffDevice, nReadLen))
				{
					bRet = false;
					SetDlgItemFormatText(IDC_STATIC_STATUS, "Disk \"%s\" check failed", fileDevices[i].GetFileName());
					goto out_close_image;
				}
			}

			m_progress_burn.OffsetPos(nReadLen);
		}
	}
	catch (...)
	{
		bRet = false;
		SetDlgItemText(IDC_STATIC_STATUS, "Read failed");
		goto out_close_image;
	}

	bRet = true;
	SetDlgItemText(IDC_STATIC_STATUS, "Device and image is match");

out_close_image:
	fileImage.Close();
out_close_device:
	CloseFiles(fileDevices, nCount);

	return bRet;
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CEmmc_burnerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CEmmc_burnerDlg::DisableAllElement(void)
{
	m_edit_file_ctrl.EnableWindow(false);
	m_button_file.EnableWindow(false);

	m_listbox_devices.EnableWindow(false);

	m_button_sel_all.EnableWindow(false);
	m_button_sel_invert.EnableWindow(false);
	m_button_desel.EnableWindow(false);

	m_button_program.EnableWindow(false);

	m_button_compare.EnableWindow(false);
	m_button_reflesh.EnableWindow(false);
}

void CEmmc_burnerDlg::EnableAllElement(void)
{
	m_edit_file_ctrl.EnableWindow(true);
	m_button_file.EnableWindow(true);

	m_listbox_devices.EnableWindow(true);

	if (m_listbox_devices.GetCount())
	{
		m_button_sel_all.EnableWindow(true);
		m_button_sel_invert.EnableWindow(true);
		m_button_desel.EnableWindow(true);
	}

	m_button_program.EnableWindow(true);
	m_button_compare.EnableWindow(true);
	m_button_reflesh.EnableWindow(true);
}

void CEmmc_burnerDlg::SetDlgItemFormatText(int nID, const char *fmt, ...)
{
	va_list ap;

	char buff[1024];

	va_start(ap, fmt);
	vsprintf(buff, fmt, ap);
	va_end(ap);

	SetDlgItemText(nID, buff);
}

int CEmmc_burnerDlg::BurnImage(void)
{
	DisableAllElement();

	if (WriteImage() && CheckDevices())
	{
		RightMessage("Program OK", "Programming Successful");
	}
	else
	{
		ErrorMessage("Program failed", "Programming failed  !!!");
	}

	EnableAllElement();

	return 0;
}

static int BurnImageFunc(void *data)
{
	CEmmc_burnerDlg *dlg = (CEmmc_burnerDlg *)data;

	return dlg->BurnImage();
}

void CEmmc_burnerDlg::OnOK()
{
	// AfxBeginThread(BurnImage, this);
	// TODO: Add extra validation here
	AfxBeginThread((AFX_THREADPROC)BurnImageFunc, this);
	// CDialog::OnOK();
}

void CEmmc_burnerDlg::OnButtonFile()
{
	// TODO: Add your control notification handler code here
	CFileDialog fileDlg(true, "bin", "Factory", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST, "Factory文件(Factory*.bin)|factory*.bin|bin文件(*.bin)|*.bin|所有文件(*)|*||");

	if (fileDlg.DoModal() == IDOK)
	{
		m_edit_file = fileDlg.GetPathName();
		UpdateData(false);
		OnChangeEditFile();
	}
}

void CEmmc_burnerDlg::OnButtonReflesh()
{
	// TODO: Add your control notification handler code here
	UpdateDevices();
}

DWORD CEmmc_burnerDlg::CRC32(CHAR buff[], DWORD dwCRC, UINT nCount)
{
	dwCRC ^= 0xFFFFFFFFL;

	while (nCount--)
	{
		dwCRC = crc_table[(dwCRC ^ (*buff++)) & 0xff] ^ (dwCRC >> 8);
	}

	return dwCRC ^ 0xFFFFFFFFL;
}

int CEmmc_burnerDlg::FileCRC32(CFile &file, DWORD &dwCRC, UINT nLenght)
{
	if (nLenght == 0)
	{
		return false;
	}

	dwCRC = 0;

	m_progress_burn.SetRange32(0, nLenght);
	m_progress_burn.SetPos(0);

	char buff[MAX_BUFF_LEN];
	UINT nReadLen;

	do {
		try
		{
			nReadLen = file.Read(buff, sizeof(buff));
			if (nReadLen > nLenght)
			{
				nReadLen = nLenght;
			}
		}
		catch (...)
		{
			return false;
		}

		dwCRC = CRC32(buff, dwCRC, nReadLen);
		nLenght -= nReadLen;
		m_progress_burn.OffsetPos(nReadLen);
	} while (nReadLen == sizeof(buff) && nLenght);

	return true;
}

int CEmmc_burnerDlg::FileCRC32(LPCTSTR lpszFileName, DWORD &dwCRC, UINT nLenght)
{
	CFile file;
	BOOL bRet;

	bRet = file.Open(lpszFileName, CFile::modeRead, NULL);
	if (bRet == false)
	{
		return false;
	}

	SetDlgItemText(IDC_STATIC_STATUS, "Calculate file CRC32 ...");

	if (nLenght == 0)
	{
		nLenght = file.GetLength();
	}

	bRet = FileCRC32(file, dwCRC, nLenght);

	file.Close();

	SetDlgItemText(IDC_STATIC_STATUS, "Calculate file CRC32 OK");

	return bRet;
}

void CEmmc_burnerDlg::OnChangeEditFile()
{
	UpdateData(true);

	AfxBeginThread((AFX_THREADPROC)UpdateImageStatusFunc, this);
}

void CEmmc_burnerDlg::OnButtonSelAll()
{
	// TODO: Add your control notification handler code here

	for (int i = m_listbox_devices.GetCount(); i >= 0; i--)
	{
		m_listbox_devices.SetCheck(i, 1);
	}
}

void CEmmc_burnerDlg::OnButtonSelInvert()
{
	// TODO: Add your control notification handler code here

	for (int i = m_listbox_devices.GetCount(); i >= 0; i--)
	{
		if (m_listbox_devices.GetCheck(i))
		{
			m_listbox_devices.SetCheck(i, 0);
		}
		else
		{
			m_listbox_devices.SetCheck(i, 1);
		}
	}
}

void CEmmc_burnerDlg::OnButtonDesel()
{
	// TODO: Add your control notification handler code here

	for (int i = m_listbox_devices.GetCount(); i >= 0; i--)
	{
		m_listbox_devices.SetCheck(i, 0);
	}
}

void CEmmc_burnerDlg::ShowMessage(LPCTSTR lpszTitle, UINT nType, LPCTSTR lpzsFormat, va_list ap)
{
	char buff[1024];

	vsprintf(buff, lpzsFormat, ap);

	MessageBox(buff, lpszTitle, nType);
}

void CEmmc_burnerDlg::ErrorMessage(LPCTSTR lpszTitle, LPCTSTR lpzsFormat, ...)
{
	va_list ap;

	va_start(ap, lpzsFormat);
	ShowMessage(lpszTitle ? lpszTitle : "Error", MB_ICONERROR, lpzsFormat, ap);
	va_end(ap);
}

void CEmmc_burnerDlg::WarningMessage(LPCTSTR lpszTitle, LPCTSTR lpzsFormat, ...)
{
	va_list ap;

	va_start(ap, lpzsFormat);
	ShowMessage(lpszTitle ? lpszTitle : "Warning", MB_ICONERROR, lpzsFormat, ap);
	va_end(ap);
}

void CEmmc_burnerDlg::RightMessage(LPCTSTR lpszTitle, LPCTSTR lpzsFormat, ...)
{
	va_list ap;

	va_start(ap, lpzsFormat);
	ShowMessage(lpszTitle ? lpszTitle : "Right", MB_ICONINFORMATION, lpzsFormat, ap);
	va_end(ap);
}

int CEmmc_burnerDlg::CompareImage(void)
{
	DisableAllElement();

	if (CheckDevices())
	{
		RightMessage("Compare is OK", "Device and image is match");
	}
	else
	{
		ErrorMessage("Compare failed", "Device and image is not match !!!");
	}

	EnableAllElement();

	return 0;
}

static int CompareImageFunc(void *data)
{
	CEmmc_burnerDlg *dlg = (CEmmc_burnerDlg *)data;

	return dlg->CompareImage();
}

void CEmmc_burnerDlg::OnButtonCompare()
{
	// TODO: Add your control notification handler code here
	AfxBeginThread((AFX_THREADPROC)CompareImageFunc, this);
}
