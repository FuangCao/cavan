// CavanDevice.h: interface for the CCavanDevice class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CAVANDEVICE_H__4FBB124C_E04E_484E_A2D4_6ABF4058AF21__INCLUDED_)
#define AFX_CAVANDEVICE_H__4FBB124C_E04E_484E_A2D4_6ABF4058AF21__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <winioctl.h>
#include <CavanFile.h>

#define DISK_DEVICE_PREFIX			"\\\\.\\PhysicalDrive"
#define PARTITION_DEVICE_PREFIX		"\\\\.\\"

#define IOCTL_DISK_UPDATE_PROPERTIES CTL_CODE(IOCTL_DISK_BASE, 0x0050, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define OPEN_DEVICE(name, mode)	\
	CreateFile(name, mode, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL)

#pragma pack(1)
class CDiskAddress
{
public:
	BYTE 	btHeaderNumber;
	BYTE	btSectorNumber		:6;
	BYTE	btCylinderNumberHSB	:2;
	BYTE	btCylinderNumberLSB;

	void ToString(CString &strString);
	DWORD ToAbsSectorNumber(DISK_GEOMETRY &geoMetry);
	static void AbsSectorNumberToDiskAddress(DWORD dwSectorNumber, CDiskAddress &daDiskAddress, DISK_GEOMETRY &geoMetry);
	static DWORD SectorCylinderAlignmentHight(DWORD dwSectorNumber, DISK_GEOMETRY &geoMetry);
	static DWORD SectorCylinderAlignmentLow(DWORD dwSectorNumber, DISK_GEOMETRY &geoMetry);
	static DWORD SectorCylinderAlignmentAuto(DWORD dwSectorNumber, DISK_GEOMETRY &geoMetry);
	static DWORD ByteToSectorAlignmentHight(ULONGLONG ulByteNumber, DISK_GEOMETRY &geoMetry);
	static DWORD ByteToSectorAlignmentLow(ULONGLONG ulByteNumber, DISK_GEOMETRY &geoMetry);
	static DWORD ByteToSectorAlignmentAuto(ULONGLONG ulByteNumber, DISK_GEOMETRY &geoMetry);
};

class CDiskPartitionTable
{
public:
	BYTE btActiveMark;
	CDiskAddress daStartAddr;
	BYTE btFileSystemMark;
	CDiskAddress daEndAddr;
	DWORD dwAbsStartSectorNumber;
	DWORD dwTotalSectorCount;

	void ToString(CString &strString);
	void SetStartSector(DWORD dwSectorNumber, DISK_GEOMETRY &geoMetry);
	void SetEndSector(DWORD dwSectorNumber, DISK_GEOMETRY &geoMetry);
	void SetPartitionSizeAlignmentHight(DWORD dwSectorCount, DISK_GEOMETRY &geoMetry);
	void SetPartitionSizeAlignmentLow(DWORD dwSectorCount, DISK_GEOMETRY &geoMetry);
	void SetPartitionSize(DWORD dwSectorCount, DISK_GEOMETRY &geoMetry);
};

class CMasterBootSector
{
public:
	BYTE btsMasterBootRecord[218];
	BYTE btsIdlesseSpace[228];
	CDiskPartitionTable dptsDiskPartTables[4];
	WORD wMagicNumber;

	void ToString(CString &strString);
};
#pragma pack()

class CCavanDevice : public CCavanFile
{
private:
	DWORD m_dwIoCtrlBytesReturned;

public:
	CCavanDevice();
	virtual ~CCavanDevice();

	BOOL OpenPhysicalDrive(int iIndex, UINT nOpenFlags);
	BOOL OpenPatition(char cIndex, UINT nOpenFlags);

	BOOL DeviceIoControlSimple(DWORD dwIoControlCode, LPVOID lpOutBuffer, DWORD nOutBufferSize);
	DWORD GetIoCtrlByteReturned(void);

	BOOL GetDeviceGeoMery(DISK_GEOMETRY &geoMetry);
	BOOL GetDeviceCapability(ULONGLONG &ullCapability);
	BOOL GetDeviceCapability(CString &strCapability);

	BOOL ReadMasterBootSector(CMasterBootSector &mbs);
	BOOL WriteMasterBootSector(CMasterBootSector &mbs);

	BOOL GetDeviceLayout(void *vpBuffer, DWORD dwBufferSize);
	static BOOL GetDeviceLayout(int iDevIndex, void *vpBuffer, DWORD dwBufferSize, DWORD &dwRetSize);
	BOOL GetPartitionInfo(PARTITION_INFORMATION *pPartInfo);
	static BOOL GetPartitionInfo(char cDevIndex, PARTITION_INFORMATION *pPartInfo);
	BOOL UpdateDiskProperties(void);
	static BOOL UpdateDiskProperties(int iDevIndex);
	static UINT GetDriveTypeFormat(const char *pbytFormat, ...);
};

#endif // !defined(AFX_CAVANDEVICE_H__4FBB124C_E04E_484E_A2D4_6ABF4058AF21__INCLUDED_)
