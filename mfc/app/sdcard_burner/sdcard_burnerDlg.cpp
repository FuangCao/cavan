// sdcard_burnerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "sdcard_burner.h"
#include "sdcard_burnerDlg.h"
#include <Cavan.h>

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
// CSdcard_burnerDlg dialog

CSdcard_burnerDlg::CSdcard_burnerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSdcard_burnerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSdcard_burnerDlg)
	m_edit_uboot_val = _T("");
	m_edit_uimage_val = _T("");
	m_edit_uramdisk_val = _T("");
	m_static_status_val = _T("");
	m_edit_if_val = _T("");
	m_edit_seek_uint = 0;
	m_edit_bs_uint = 0;
	m_edit_count_uint = 0;
	m_edit_skip_uint = 0;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSdcard_burnerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSdcard_burnerDlg)
	DDX_Control(pDX, IDC_COMBO_SKIP, m_combo_skip_ctrl);
	DDX_Control(pDX, IDC_COMBO_SEEK, m_combo_seek_ctrl);
	DDX_Control(pDX, IDC_COMBO_COUNT, m_combo_count_ctrl);
	DDX_Control(pDX, IDC_COMBO_BS, m_combo_bs_ctrl);
	DDX_Control(pDX, IDC_PROGRESS_BURN, m_progress_burn_ctrl);
	DDX_Control(pDX, IDC_COMBO_DEVICE, m_combo_device_ctrl);
	DDX_Control(pDX, IDC_COMBO_UBOOT, m_combo_uboot_ctrl);
	DDX_Text(pDX, IDC_EDIT_UBOOT, m_edit_uboot_val);
	DDX_Text(pDX, IDC_EDIT_UIMAGE, m_edit_uimage_val);
	DDX_Text(pDX, IDC_EDIT_URAMDISK, m_edit_uramdisk_val);
	DDX_Text(pDX, IDC_STATIC_STATUS, m_static_status_val);
	DDX_Text(pDX, IDC_EDIT_IF, m_edit_if_val);
	DDX_Text(pDX, IDC_EDIT_SEEK, m_edit_seek_uint);
	DDX_Text(pDX, IDC_EDIT_BS, m_edit_bs_uint);
	DDX_Text(pDX, IDC_EDIT_COUNT, m_edit_count_uint);
	DDX_Text(pDX, IDC_EDIT_SKIP, m_edit_skip_uint);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSdcard_burnerDlg, CDialog)
	//{{AFX_MSG_MAP(CSdcard_burnerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_UBOOT, OnButtonUboot)
	ON_BN_CLICKED(IDC_BUTTON_UIMAGE, OnButtonUimage)
	ON_BN_CLICKED(IDC_BUTTON_URAMDISK, OnButtonUramdisk)
	ON_BN_CLICKED(IDC_BUTTON_FACTORY, OnButtonFactory)
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, OnButtonRefresh)
	ON_BN_CLICKED(IDC_BUTTON_IF, OnButtonIf)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSdcard_burnerDlg message handlers

BOOL CSdcard_burnerDlg::OnInitDialog()
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
	m_combo_uboot_ctrl.AddString("u-boot");
	m_combo_uboot_ctrl.AddString("factory");
	m_combo_uboot_ctrl.AddString("u-boot-no-padding");
	m_combo_uboot_ctrl.SetCurSel(0);

	const char *pcUnits[] = {"B", "KB", "MB", "GB", "TB"};

	for (int i = 0; i < ARRAY_SIZE(pcUnits); i++)
	{
		m_combo_bs_ctrl.AddString(pcUnits[i]);
		m_combo_count_ctrl.AddString(pcUnits[i]);
		m_combo_seek_ctrl.AddString(pcUnits[i]);
		m_combo_skip_ctrl.AddString(pcUnits[i]);
	}

	m_combo_bs_ctrl.SetCurSel(0);
	m_combo_count_ctrl.SetCurSel(0);
	m_combo_seek_ctrl.SetCurSel(0);
	m_combo_skip_ctrl.SetCurSel(0);

	UpdateDevice();

	UpdateData(false);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSdcard_burnerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSdcard_burnerDlg::OnPaint()
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
HCURSOR CSdcard_burnerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CSdcard_burnerDlg::OnButtonUboot()
{
	// TODO: Add your control notification handler code here
	CString strUbootType;

	UpdateData(true);

	COMBO_GET_TEXT(m_combo_uboot_ctrl, strUbootType);

	CFileDialog fileDlg(true, "bin", strUbootType,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST,
		strUbootType + "文件" + "|" + strUbootType + "*.bin||");

	if (fileDlg.DoModal() == IDOK)
	{
		m_edit_uboot_val = fileDlg.GetPathName();
		UpdateData(false);
	}
}

void CSdcard_burnerDlg::OnButtonUimage()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);

	CFileDialog fileDlg(true, NULL, "uImage", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST, "uImage文件|uImage*||");

	if (fileDlg.DoModal() == IDOK)
	{
		m_edit_uimage_val = fileDlg.GetPathName();
		UpdateData(false);
	}
}

void CSdcard_burnerDlg::OnButtonUramdisk()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);

	CFileDialog fileDlg(true, "img", "uramdisk", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST, "uramdisk/busybox文件|uramdisk*.img;busybox*.img||");

	if (fileDlg.DoModal() == IDOK)
	{
		m_edit_uramdisk_val = fileDlg.GetPathName();
		UpdateData(false);
	}
}

void CSdcard_burnerDlg::OnOK()
{
	// TODO: Add extra validation here
	int iTargetDevIndex;

	iTargetDevIndex = GetTargetDeviceIndex();
	if (iTargetDevIndex < 0)
	{
		return;
	}

	if (WriteImages(iTargetDevIndex) == false)
	{
		return;
	}

	CCavanDebug::RightMessage(m_hWnd, "Burn SDCard Successfully");
	// CDialog::OnOK();
}

void CSdcard_burnerDlg::PartInfoToString(PPARTITION_INFORMATION pPartInfo, CString &strPartInfo)
{
#if 0
	strPartInfo.Format("BootIndicator = 0x%08x, HiddenSectors = 0x%08x, \
		PartitionNumber = 0x%08x, HiddenSectors = 0x%08x, \
		RecognizedPartition = 0x%08x, RewritePartition = 0x%08x",
		pPartInfo->BootIndicator, pPartInfo->HiddenSectors,
		pPartInfo->PartitionNumber, pPartInfo->PartitionType,
		pPartInfo->RecognizedPartition, pPartInfo->RewritePartition);
#else
	char buff[10];

	strPartInfo.Empty();

	unsigned char *pbytTemp = (unsigned char *)pPartInfo;

	for (int i = 0; i < sizeof(PARTITION_INFORMATION); i++)
	{
		sprintf(buff, "%02x", pbytTemp[i]);
		strPartInfo += buff;
	}

	// strPartInfo = "123456";
#endif
}

BOOL CSdcard_burnerDlg::FormatDevicePartitions(int iDevIndex)
{
	DWORD dwDeviceLayoutLength;
	char bytsDeviceLayout[1024];

	m_static_status_val = "GetDeviceLayout ...";
	UpdateData(false);

	if (CCavanDevice::GetDeviceLayout(iDevIndex, bytsDeviceLayout, sizeof(bytsDeviceLayout), dwDeviceLayoutLength) == false)
	{
		CCavanDebug::ErrorMessage(m_hWnd, "Get Device%d Layout Failed", iDevIndex);
		return false;
	}

	DWORD dwDriveStringLength;
	char bytsDriveStrings[1024];
	dwDriveStringLength = GetLogicalDriveStrings(sizeof(bytsDriveStrings), bytsDriveStrings);

	LPSTR lpTargetDevice, lpDriveStringsEnd;
	PARTITION_INFORMATION PartInfo;
	PDRIVE_LAYOUT_INFORMATION pDeviceLayoutInfo;
	CString strTemp;

	pDeviceLayoutInfo = (PDRIVE_LAYOUT_INFORMATION)bytsDeviceLayout;

	m_static_status_val.Format("Find First Partition From Device%d ...", iDevIndex);
	UpdateData(false);

	for (lpTargetDevice = bytsDriveStrings, lpDriveStringsEnd = bytsDriveStrings + dwDriveStringLength;
		lpTargetDevice < lpDriveStringsEnd; lpTargetDevice += strlen(lpTargetDevice) + 1)
	{
		if (GetDriveType(lpTargetDevice) != DRIVE_REMOVABLE)
		{
			continue;
		}

		if (CCavanDevice::GetPartitionInfo(lpTargetDevice[0], &PartInfo) == false)
		{
			continue;
		}
#if 0
		CString strPart1, strPart2;
		PartInfoToString(pDeviceLayoutInfo->PartitionEntry, strPart1);
		PartInfoToString(&PartInfo, strPart2);
		MessageBox(strPart1 + "\n\n" + strPart2 + "\n\n" + lpTargetDevice);
#endif
		if (memcmp(pDeviceLayoutInfo->PartitionEntry, &PartInfo, sizeof(PartInfo) - 4) == 0)
		{
			goto out_format_partition;
		}
	}

	CCavanDebug::WarningMessage(m_hWnd, "Find First Partitions From Device%d Failed", iDevIndex);
	return false;

out_format_partition:
	strTemp.Format("Do You Want To Format Partition \"%s\"?", lpTargetDevice);

	if (MessageBox(strTemp, "Formate Partition", MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES)
	{
		strTemp.Format("format %c: /q /y /x /fs:fat32 /v:CFA8888", lpTargetDevice[0]);

		m_static_status_val.Format("Format Partition \"%s\" ...", lpTargetDevice);
		UpdateData(false);

		system(strTemp);
	}

	return true;
}

void CSdcard_burnerDlg::OnButtonFactory()
{
	// TODO: Add your control notification handler code here
	if (MessageBox("Burn Factory SDCard Will Repartition The SDCard.\nDo You Want To Continue?",
		"Burn Factory SDCard", MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) != IDYES)
	{
		return;
	}

	int iDevIndex;

	iDevIndex = GetTargetDeviceIndex();
	if (iDevIndex < 0)
	{
		return;
	}

	if (WritePartitionTable(iDevIndex, MB(64)) == false)
	{
		return;
	}

	if (WriteImages(iDevIndex) == false)
	{
		return;
	}

	if (WritePartitionTable(iDevIndex, MB(64)) == false)
	{
		return;
	}

	if (FormatDevicePartitions(iDevIndex) == false)
	{
		return;
	}

	CCavanDebug::RightMessage(m_hWnd, "Create Factory SDCard Successfully");
}

void CSdcard_burnerDlg::OnButtonRefresh()
{
	// TODO: Add your control notification handler code here
	UpdateDevice();
}

void CSdcard_burnerDlg::UpdateDevice(void)
{
	CString strTemp, strSize;
	ULONGLONG ullDeviceSize;
	CCavanDevice cvnDevice;

	m_combo_device_ctrl.ResetContent();

	for (int i = 1; i <= 26; i++)
	{
		if (cvnDevice.OpenPhysicalDrive(i, 0) == false)
		{
			continue;
		}

		if(cvnDevice.GetDeviceCapability(ullDeviceSize) && (ullDeviceSize >> 30) < 10)
		{
			CCavanText::SizeToText(ullDeviceSize, strSize);
			strTemp.Format("%d <> %s", i, strSize);
			m_combo_device_ctrl.AddString(strTemp);
		}

		cvnDevice.Close();
	}

	m_combo_device_ctrl.SetCurSel(m_combo_device_ctrl.GetCount() - 1);
}

int CSdcard_burnerDlg::BurnNormalImage(CCavanDevice &cvnDevice)
{
	UpdateData(true);

	if (m_edit_if_val.IsEmpty())
	{
		return true;
	}

	CString strTemp;
	LONG lBs, lCount, lSeek, lSkip;

	COMBO_GET_TEXT(m_combo_bs_ctrl, strTemp);
	lBs = m_edit_bs_uint * CCavanText::UnitToSize(strTemp[0]);

	COMBO_GET_TEXT(m_combo_count_ctrl, strTemp);
	lCount = m_edit_count_uint * lBs * CCavanText::UnitToSize(strTemp[0]);

	COMBO_GET_TEXT(m_combo_seek_ctrl, strTemp);
	lSeek = m_edit_seek_uint * lBs * CCavanText::UnitToSize(strTemp[0]);

	COMBO_GET_TEXT(m_combo_skip_ctrl, strTemp);
	lSkip = m_edit_skip_uint * lBs * CCavanText::UnitToSize(strTemp[0]);

	m_static_status_val.Format("Burn Image \"%s\"", m_edit_if_val);
	UpdateData(false);

	if (cvnDevice.CopyFrom(m_progress_burn_ctrl, m_edit_if_val, lSkip, lSeek, lCount) == false)
	{
		MessageBox(cvnDevice.strLog);
		CCavanDebug::ErrorMessage(m_hWnd, "Burn Image \"%s\" To Device Failed", m_edit_if_val);
		return false;
	}

	return true;
}

void CSdcard_burnerDlg::OnButtonIf()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);

	CFileDialog fileDlg(true, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST, NULL);

	if (fileDlg.DoModal() == IDOK)
	{
		m_edit_if_val = fileDlg.GetPathName();
		UpdateData(false);
	}
}

int CSdcard_burnerDlg::GetTargetDeviceIndex(void)
{
	if (m_combo_device_ctrl.GetCount() < 0)
	{
		CCavanDebug::WarningMessage(m_hWnd, "No Device Find");
		return -1;
	}

	CString strDevice;

	COMBO_GET_TEXT(m_combo_device_ctrl, strDevice);

	return strDevice[0] - '0';
}

BOOL CSdcard_burnerDlg::WritePartitionTable(int iDevIndex, ULONGLONG ullStartByte)
{

	CCavanDevice cvnDevice;

	if (cvnDevice.OpenPhysicalDrive(iDevIndex, CFile::modeReadWrite) == false)
	{
		CCavanDebug::ErrorMessage(m_hWnd, "Open Device%d Failed", iDevIndex);
		return false;
	}

	CMasterBootSector mbs;
	BOOL bRet = false;

	if (cvnDevice.ReadMasterBootSector(mbs) == false)
	{
		CCavanDebug::ErrorMessage(m_hWnd, "Read Master Boot Sectort Failed");
		goto out_close_device;
	}

	DISK_GEOMETRY geoMetry;

	if (cvnDevice.GetDeviceGeoMery(geoMetry) == false)
	{
		CCavanDebug::ErrorMessage(m_hWnd, "Get GeoMery Failed");
		goto out_close_device;
	}

	memset(mbs.dptsDiskPartTables, 0, sizeof(CDiskPartitionTable) * 4);
	mbs.wMagicNumber = 0xAA55;

	DWORD dwStartSector;

	dwStartSector = CDiskAddress::ByteToSectorAlignmentHight(ullStartByte, geoMetry);
	mbs.dptsDiskPartTables[0].SetStartSector(dwStartSector, geoMetry);
	mbs.dptsDiskPartTables[0].btFileSystemMark = 0x06;

	DWORD dwTotalSector;

	dwTotalSector = (DWORD)(geoMetry.Cylinders.QuadPart * geoMetry.TracksPerCylinder * geoMetry.SectorsPerTrack);
	mbs.dptsDiskPartTables[0].SetPartitionSize(dwTotalSector - dwStartSector, geoMetry);

	// mbs.ToString(strDevice);
	// MessageBox(strDevice);

	if (cvnDevice.WriteMasterBootSector(mbs) == false)
	{
		CCavanDebug::ErrorMessage(m_hWnd, "Write Master Boot Sector Failed");
		goto out_close_device;
	}

	if (cvnDevice.UpdateDiskProperties() == false)
	{
		CCavanDebug::ErrorMessage(m_hWnd, "UpdateDiskProperties Failed");
		goto out_close_device;
	}

	// CCavanDebug::RightMessage(m_hWnd, "Write Master Boot Sector Success");
	bRet = true;

out_close_device:
	cvnDevice.Close();

	return bRet;
}

BOOL CSdcard_burnerDlg::WriteImages(int iDevIndex)
{
	UpdateData(true);

	if (m_combo_device_ctrl.GetCount() <= 0)
	{
		CCavanDebug::ErrorMessage(m_hWnd, "No Device Find");
		return false;
	}

	CCavanDevice cvnDevice;

	if (cvnDevice.OpenPhysicalDrive(iDevIndex, CFile::modeWrite) == false)
	{
		CCavanDebug::ErrorMessage(m_hWnd, "Open Device%d Failed", iDevIndex);
		return false;
	}

	BOOL bRet = false;

	if (!m_edit_uboot_val.IsEmpty())
	{
		m_static_status_val.Format("Burn Image \"%s\"", m_edit_uboot_val);
		UpdateData(false);

		if (m_combo_uboot_ctrl.GetCurSel() < 2)
		{
			if (cvnDevice.CopyFrom(m_progress_burn_ctrl, m_edit_uboot_val, KB(1), KB(1), 0) == false)
			{
				CCavanDebug::ErrorMessage(m_hWnd, "Copy File From \"%s\" Failed", m_edit_uboot_val);
				goto out_close_device;
			}
		}
		else
		{
			if (cvnDevice.CopyFrom(m_progress_burn_ctrl, m_edit_uboot_val, 0, KB(1), 0) == false)
			{
				CCavanDebug::ErrorMessage(m_hWnd, "Copy File From \"%s\" Failed", m_edit_uboot_val);
				goto out_close_device;
			}
		}
	}

	if (!m_edit_uimage_val.IsEmpty())
	{
		m_static_status_val.Format("Burn Image \"%s\"", m_edit_uimage_val);
		UpdateData(false);

		if (cvnDevice.CopyFrom(m_progress_burn_ctrl, m_edit_uimage_val, 0, MB(1), 0) == false)
		{
			CCavanDebug::ErrorMessage(m_hWnd, "Copy File From \"%s\" Failed", m_edit_uimage_val);
			goto out_close_device;
		}
	}

	if (!m_edit_uramdisk_val.IsEmpty())
	{
		m_static_status_val.Format("Burn Image \"%s\"", m_edit_uramdisk_val);
		UpdateData(false);

		if (cvnDevice.CopyFrom(m_progress_burn_ctrl, m_edit_uramdisk_val, 0, MB(5), 0) == false)
		{
			CCavanDebug::ErrorMessage(m_hWnd, "Copy File From \"%s\" Failed", m_edit_uramdisk_val);
			goto out_close_device;
		}
	}

	if (BurnNormalImage(cvnDevice) == false)
	{
		goto out_close_device;
	}

	bRet = true;

out_close_device:
	cvnDevice.Close();
	return bRet;
}
