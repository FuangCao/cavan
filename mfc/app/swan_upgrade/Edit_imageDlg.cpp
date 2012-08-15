// Edit_imageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "swan_upgrade.h"
#include "Edit_imageDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEdit_imageDlg dialog


CEdit_imageDlg::CEdit_imageDlg(struct CSwanImage *pCurrentImage, CWnd* pParent /*=NULL*/)
	: CDialog(CEdit_imageDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEdit_imageDlg)
	m_edit_path = _T("");
	//}}AFX_DATA_INIT

	this->pCurrentImage = pCurrentImage;
}


void CEdit_imageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEdit_imageDlg)
	DDX_Control(pDX, IDC_COMBO_TYPE, m_combo_type);
	DDX_Control(pDX, IDC_COMBO_OFFSET, m_combo_offset);
	DDX_Control(pDX, IDC_COMBO_MINOR, m_combo_minor);
	DDX_Control(pDX, IDC_COMBO_MAJOR, m_combo_major);
	DDX_Control(pDX, IDC_COMBO_DEVICE, m_combo_device);
	DDX_Text(pDX, IDC_EDIT_PATH, m_edit_path);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEdit_imageDlg, CDialog)
	//{{AFX_MSG_MAP(CEdit_imageDlg)
	ON_CBN_SELCHANGE(IDC_COMBO_TYPE, OnSelchangeComboType)
	ON_BN_CLICKED(IDC_BUTTON_PATH, OnButtonPath)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEdit_imageDlg message handlers

void CEdit_imageDlg::OnOK()
{
	// TODO: Add extra validation here
	UpdateData(true);

	if (m_edit_path.IsEmpty())
	{
		MessageBox("��ѡ��Image��·��");
		return;
	}

	CString rTemp;

	m_combo_type.GetLBText(m_combo_type.GetCurSel(), rTemp);

	struct CSwanImage *pImage = CSwan_upgradeApp::GetImageInfo(rTemp);

	if (pImage == NULL)
	{
		return;
	}

	CFile file;

	if (file.Open(m_edit_path, 0, NULL) == false)
	{
		rTemp.Format("�ļ�\"%s\"������", m_edit_path);
		MessageBox(rTemp);
		return;
	}

	pImage->dwLength = file.GetLength();

	file.Close();

	m_combo_device.GetLBText(m_combo_device.GetCurSel(), rTemp);
	strncpy(pImage->cDevicePath, rTemp, MAX_PATH_LEN);

	m_combo_major.GetLBText(m_combo_major.GetCurSel(), rTemp);
	pImage->nMajor = TextToValue(rTemp);

	m_combo_minor.GetLBText(m_combo_minor.GetCurSel(), rTemp);
	pImage->nMinor = TextToValue(rTemp);

	m_combo_offset.GetLBText(m_combo_offset.GetCurSel(), rTemp);
	pImage->dwOffset = TextToSize(rTemp);

	CDialog::OnOK();
}

BOOL CEdit_imageDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here
	int i;

	for (i = 0; i < MAX_IMAGE_COUNT; i++)
	{
		if (CSwan_upgradeApp::imgSwanImages[i].dwLength && pCurrentImage != (CSwan_upgradeApp::imgSwanImages + i))
		{
			continue;
		}

		if (CSwan_upgradeApp::imgSwanImages[i].cFileName[0])
		{
			m_combo_type.AddString(CSwan_upgradeApp::imgSwanImages[i].cFileName);
		}
	}

	if (m_combo_type.GetCount() == 0)
	{
		MessageBox("û�п���ӵ�Image");
		return false;
	}

	m_combo_major.AddString("179");

	CString rTemp;
	for (i = 0; i < 16; i++)
	{
		rTemp.Format("%d", i);
		m_combo_minor.AddString(rTemp);
	}

	m_combo_offset.AddString("0B");
	m_combo_offset.AddString("1KB");
	m_combo_offset.AddString("1MB");
	m_combo_offset.AddString("4MB");
	m_combo_offset.AddString("12MB");
	m_combo_offset.AddString("14MB");
	m_combo_offset.AddString("30MB");

	int j;

	for (i = 0; i < 2; i++)
	{
		rTemp.Format("/dev/mmcblk%d", i);
		m_combo_device.AddString(rTemp);

		for (j = 1; j < 9; j++)
		{
			rTemp.Format("/dev/mmcblk%dp%d", i, j);
			m_combo_device.AddString(rTemp);
		}
	}

	if (pCurrentImage == NULL)
	{
		SetWindowText(_T("���Image"));
		m_combo_type.SetCurSel(0);
	}
	else
	{
		SetWindowText(_T("�༭Image"));
		m_combo_type.SelectString(-1, pCurrentImage->cFileName);
	}

	OnSelchangeComboType();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEdit_imageDlg::OnSelchangeComboType()
{
	// TODO: Add your control notification handler code here
	CString rType;

	m_combo_type.GetLBText(m_combo_type.GetCurSel(), rType);
	struct CSwanImage *pCurrentImage = CSwan_upgradeApp::GetImageInfo(rType);

	if (pCurrentImage)
	{
		SelectImage(pCurrentImage);
	}
}

void CEdit_imageDlg::SelectImage(struct CSwanImage *pCurrentImage)
{
	this->pCurrentImage = pCurrentImage;

	pCurrentImage->dwLength = 0;

	m_combo_device.SelectString(-1, pCurrentImage->cDevicePath);
	m_combo_offset.SelectString(-1, SizeToText(pCurrentImage->dwOffset));

	CString rTemp;

	rTemp.Format("%d", pCurrentImage->nMajor);
	m_combo_major.SelectString(-1, rTemp);

	rTemp.Format("%d", pCurrentImage->nMinor);
	m_combo_minor.SelectString(-1, rTemp);
}

CString &CEdit_imageDlg::SizeToText(UINT nSize)
{
	static CString cpText;
	CString rTemp;
	UINT nTemp;

	if (nSize == 0)
	{
		cpText = "0B";
		return cpText;
	}

	cpText.Empty();

	nTemp = (nSize >> 30) & 0x3FF;
	if (nTemp)
	{
		rTemp.Format("%dG", nTemp);
		cpText += rTemp;
	}

	nTemp = (nSize >> 20) & 0x3FF;
	if (nTemp)
	{
		rTemp.Format("%dM", nTemp);
		cpText += rTemp;
	}

	nTemp = (nSize >> 10) & 0x3FF;
	if (nTemp)
	{
		rTemp.Format("%dK", nTemp);
		cpText += rTemp;
	}

	nTemp = nSize & 0x3FF;
	if (nTemp)
	{
		rTemp.Format("%dB", nTemp);
	}
	else
	{
		rTemp = "B";
	}

	cpText += rTemp;

	return cpText;
}

UINT CEdit_imageDlg::TextToSize(const char cpText[])
{
	UINT nSize;
	UINT nTemp;

	nSize = 0;

	while (cpText[0])
	{
		if (cpText[0] >= '0' && cpText[0] <= '9')
		{
			nTemp = 0;

			do {
				nTemp *= 10;
				nTemp += *cpText++ - '0';
			} while (cpText[0] >= '0' && cpText[0] <= '9');
		}
		else
		{
			nSize += nTemp * UnitsToSize(*cpText++);
			nTemp = 0;
		}
	}

	return nSize;
}

UINT CEdit_imageDlg::UnitsToSize(char cUnit)
{
	switch (cUnit)
	{
	case 'b':
	case 'B':
		return 1;
	case 'k':
	case 'K':
		return 1 << 10;
	case 'm':
	case 'M':
		return 1 << 20;
	case 'g':
	case 'G':
		return 1 << 30;
	default:
		return 0;
	}

	return 0;
}

UINT CEdit_imageDlg::TextToValue(const char cpText[])
{
	UINT nValue;

	nValue = 0;

	while (cpText[0])
	{
		nValue *= 10;
		nValue += *cpText++ - '0';
	}

	return nValue;
}

CString &CEdit_imageDlg::ValueToText(UINT nValue)
{
	static CString rText;

	rText.Format("%d", nValue);

	return rText;
}

struct CFileDialogInfo
{
	LPCTSTR lpszImgType;
	LPCTSTR lpszDefExt;
	LPCTSTR lpszFileName;
	LPCTSTR lpszFilter;
};

struct CFileDialogInfo fileDlgInfos[] = {
	{"u-boot-no-padding.bin", "bin", "u-boot-no-padding", "uboot�ļ�|u-boot-no-padding*.bin||"},
	{"uImage", NULL, "uImage", "uImage�ļ�|uImage*||"},
	{"uramdisk.img", "img", "uramdisk", "uramdisk�ļ�|uramdisk*.img||"},
	{"logo.bmp", "bmp", "logo", "logo�ļ�|logo*.bmp||"},
	{"busybox.img", "img", "busybox", "busybox�ļ�|busybox*.img||"},
	{"charge.bmps", "bmps", "charge", "charge�ļ�|charge*.bmps||"},
	{"system.img", "img", "system", "system�ļ�|system*.img||"},
	{"recovery.img", "img", "recovery", "recovery�ļ�|recovery*.img||"},
	{"userdata.img", "img", "userdata", "userdata�ļ�|userdata*.img||"},
};

void CEdit_imageDlg::OnButtonPath()
{
	// TODO: Add your control notification handler code here
	int i;
	CString rImageType;

	m_combo_type.GetLBText(m_combo_type.GetCurSel(), rImageType);
	for (i = 0; i < ARRAY_SIZE(fileDlgInfos); i++)
	{
		if (rImageType.Compare(fileDlgInfos[i].lpszImgType) == 0)
		{
			break;
		}
	}

	if (i >= ARRAY_SIZE(fileDlgInfos))
	{
		return;
	}

	CFileDialog fileDlg(true, fileDlgInfos[i].lpszDefExt, fileDlgInfos[i].lpszFileName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST, fileDlgInfos[i].lpszFilter);

	if (fileDlg.DoModal() == IDOK)
	{
		m_edit_path = fileDlg.GetPathName();
		UpdateData(false);
	}
}
