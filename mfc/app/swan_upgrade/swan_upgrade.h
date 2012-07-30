// swan_upgrade.h : main header file for the SWAN_UPGRADE application
//

#if !defined(AFX_SWAN_UPGRADE_H__065DDA42_F764_47F8_B027_D39D9919693E__INCLUDED_)
#define AFX_SWAN_UPGRADE_H__065DDA42_F764_47F8_B027_D39D9919693E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

#define KB(a)					((a) << 10)
#define MB(a)					((a)  << 20)
#define ARRAY_SIZE(a)			(sizeof(a) / sizeof(a[0]))
#define MAX_NAME_LEN			128
#define MAX_PATH_LEN			256
#define MAX_MD5SUM_LEN			36
#define MAX_VERSION_LEN			128
#define MAX_IMAGE_COUNT			9

#define UPGRADE_PROGRAM_NAME	"swan_upgrade"
#define TEMP_DEVICE_PATH		"/tmp/swan_device"
#define TARGET_DEVICE			"/dev/mmcblk0"
#define FIRST_MINOR				0
#define BACKUP_HEADER_BIN		"/etc/swan/header.bin"
#define CHECK_PATTERN			"eavoo_swan_i600"

/////////////////////////////////////////////////////////////////////////////
// CSwan_upgradeApp:
// See swan_upgrade.cpp for the implementation of this class
//

struct CSwanImage
{
	UINT nMajor;
	UINT nMinor;
	DWORD dwOffset;
	DWORD dwLength;
	char cFileName[MAX_NAME_LEN];
	char cDevicePath[MAX_PATH_LEN];
};

struct CPackageInfo
{
	UINT image_count;
	char cVersion[MAX_VERSION_LEN];
};

struct CFileInfo
{
	UINT nBuildTime;
	DWORD dwOffset;
	char cMd5Sum[MAX_MD5SUM_LEN];
	char cCheckPattern[sizeof(CHECK_PATTERN)];
};

class CSwan_upgradeApp : public CWinApp
{
public:
	CSwan_upgradeApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSwan_upgradeApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSwan_upgradeApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	static struct CSwanImage imgSwanImages[MAX_IMAGE_COUNT];
	static struct CSwanImage *GetImageInfo(const CString &rImageName);
	static struct CSwanImage *GetImageInfo(int iIndex);
	static int GetImageIndex(struct CSwanImage *pImage);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SWAN_UPGRADE_H__065DDA42_F764_47F8_B027_D39D9919693E__INCLUDED_)
