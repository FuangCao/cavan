// swan_upgrade.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "swan_upgrade.h"
#include "swan_upgradeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSwan_upgradeApp

BEGIN_MESSAGE_MAP(CSwan_upgradeApp, CWinApp)
	//{{AFX_MSG_MAP(CSwan_upgradeApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSwan_upgradeApp construction

CSwan_upgradeApp::CSwan_upgradeApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSwan_upgradeApp object

CSwan_upgradeApp theApp;

struct CSwanImage CSwan_upgradeApp::imgSwanImages[] = {
	{179, FIRST_MINOR + 5, 0, 0, "userdata.img", TARGET_DEVICE "p5"},
	{179, FIRST_MINOR + 4, 0, 0, "recovery.img", TARGET_DEVICE "p4"},
	{179, FIRST_MINOR + 2, 0, 0, "system.img", TARGET_DEVICE "p2"},
	{179, FIRST_MINOR + 0, MB(30), 0, "charge.bmps", TARGET_DEVICE},
	{179, FIRST_MINOR + 0, MB(14), 0, "busybox.img", TARGET_DEVICE},
	{179, FIRST_MINOR + 0, MB(12), 0, "logo.bmp", TARGET_DEVICE},
	{179, FIRST_MINOR + 0, MB(5), 0, "uramdisk.img", TARGET_DEVICE},
	{179, FIRST_MINOR + 0, MB(1), 0, "uImage", TARGET_DEVICE},
	{179, FIRST_MINOR + 0, KB(1), 0, "u-boot-no-padding.bin", TARGET_DEVICE}
};

/////////////////////////////////////////////////////////////////////////////
// CSwan_upgradeApp initialization

BOOL CSwan_upgradeApp::InitInstance()
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

	CSwan_upgradeDlg dlg;
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

struct CSwanImage *CSwan_upgradeApp::GetImageInfo(const CString &rImageName)
{
	for (int i = 0; i < MAX_IMAGE_COUNT; i++)
	{
		if (rImageName.Compare(imgSwanImages[i].cFileName) == 0)
		{
			return imgSwanImages + i;
		}
	}

	return NULL;
}

struct CSwanImage *CSwan_upgradeApp::GetImageInfo(int iIndex)
{
	if (iIndex < 0)
	{
		return NULL;
	}

	return imgSwanImages + iIndex;
}

int CSwan_upgradeApp::GetImageIndex(struct CSwanImage *pImage)
{
	if (pImage < imgSwanImages)
	{
		return -1;
	}

	return pImage - imgSwanImages;
}