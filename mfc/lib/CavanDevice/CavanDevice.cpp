// CavanDevice.cpp: implementation of the CCavanDevice class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Cavan.h>
#include <CavanDevice.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void CDiskAddress::ToString(CString &strString)
{
	strString.Format("cylinder = %d, header = %d, sector = %d",
		btCylinderNumberHSB << 8 | btCylinderNumberLSB, btHeaderNumber, btSectorNumber);
}

DWORD CDiskAddress::ToAbsSectorNumber(DISK_GEOMETRY &geoMetry)
{
	DWORD dwSectorsPerCyliner;

	dwSectorsPerCyliner = geoMetry.TracksPerCylinder * geoMetry.SectorsPerTrack;

	DWORD dwAbsSector;

	dwAbsSector = (btCylinderNumberHSB << 8 | btCylinderNumberLSB) * dwSectorsPerCyliner;
	dwAbsSector += btHeaderNumber * geoMetry.SectorsPerTrack;
	dwAbsSector += btSectorNumber - 1;

	return dwSectorsPerCyliner;
}

void CDiskAddress::AbsSectorNumberToDiskAddress(DWORD dwSectorNumber, CDiskAddress &daDiskAddress, DISK_GEOMETRY &geoMetry)
{
	DWORD dwSectorsPerCyliner;

	dwSectorsPerCyliner = geoMetry.TracksPerCylinder * geoMetry.SectorsPerTrack;

	DWORD wCylinderNumber;

	wCylinderNumber = dwSectorNumber / dwSectorsPerCyliner;
	daDiskAddress.btCylinderNumberHSB = (wCylinderNumber >> 8) & 0x03;
	daDiskAddress.btCylinderNumberLSB = (UCHAR)(wCylinderNumber & 0xFF);
	dwSectorNumber %= dwSectorsPerCyliner;

	daDiskAddress.btHeaderNumber = (UCHAR)(dwSectorNumber / geoMetry.SectorsPerTrack);
	dwSectorNumber %= geoMetry.SectorsPerTrack;

	daDiskAddress.btSectorNumber = dwSectorNumber + 1;
}

DWORD CDiskAddress::SectorCylinderAlignmentHight(DWORD dwSectorNumber, DISK_GEOMETRY &geoMetry)
{
	DWORD dwSectorPerCylinder;

	dwSectorPerCylinder = geoMetry.TracksPerCylinder * geoMetry.SectorsPerTrack;

	return (dwSectorNumber + dwSectorPerCylinder) / dwSectorPerCylinder * dwSectorPerCylinder;
}

DWORD CDiskAddress::SectorCylinderAlignmentLow(DWORD dwSectorNumber, DISK_GEOMETRY &geoMetry)
{
	DWORD dwSectorPerCylinder;

	dwSectorPerCylinder = geoMetry.TracksPerCylinder * geoMetry.SectorsPerTrack;

	return dwSectorNumber / dwSectorPerCylinder * dwSectorPerCylinder;
}

DWORD CDiskAddress::SectorCylinderAlignmentAuto(DWORD dwSectorNumber, DISK_GEOMETRY &geoMetry)
{
	DWORD dwSectorPerCylinder;

	dwSectorPerCylinder = geoMetry.TracksPerCylinder * geoMetry.SectorsPerTrack;

	if (dwSectorNumber % dwSectorPerCylinder > (dwSectorPerCylinder >> 1))
	{
		return dwSectorNumber / dwSectorPerCylinder * dwSectorPerCylinder +  dwSectorPerCylinder;
	}
	else
	{
		return dwSectorNumber / dwSectorPerCylinder  * dwSectorPerCylinder;
	}
}

DWORD CDiskAddress::ByteToSectorAlignmentHight(ULONGLONG ulByteNumber, DISK_GEOMETRY &geoMetry)
{
	return (DWORD)((ulByteNumber + geoMetry.BytesPerSector) / geoMetry.BytesPerSector);
}

DWORD CDiskAddress::ByteToSectorAlignmentLow(ULONGLONG ulByteNumber, DISK_GEOMETRY &geoMetry)
{
	return (DWORD)(ulByteNumber / geoMetry.BytesPerSector);
}

DWORD CDiskAddress::ByteToSectorAlignmentAuto(ULONGLONG ulByteNumber, DISK_GEOMETRY &geoMetry)
{
	if (ulByteNumber % geoMetry.BytesPerSector > (geoMetry.BytesPerSector >> 1))
	{
		return (DWORD)(ulByteNumber / geoMetry.BytesPerSector + 1);
	}
	else
	{
		return (DWORD)(ulByteNumber / geoMetry.BytesPerSector);
	}
}

// ==========================================================================================

void CDiskPartitionTable::ToString(CString &strString)
{
	strString.Format("ActiveMark = 0x%02x\n", btActiveMark);

	CString strTemp;
	CString daAddress;

	daStartAddr.ToString(daAddress);
	strTemp.Format("StartAddr = %s\n", daAddress);
	strString += strTemp;

	strTemp.Format("FileSystemMark = 0x%02x\n", btFileSystemMark);
	strString += strTemp;

	daEndAddr.ToString(daAddress);
	strTemp.Format("EndAddr = %s\n", daAddress);
	strString += strTemp;

	strTemp.Format("AbsStartSectorNumber = %d\n", dwAbsStartSectorNumber);
	strString += strTemp;

	strTemp.Format("TotalSectorCount = %d", dwTotalSectorCount);
	strString += strTemp;
}

void CDiskPartitionTable::SetStartSector(DWORD dwSectorNumber, DISK_GEOMETRY &geoMetry)
{
	dwAbsStartSectorNumber = dwSectorNumber;
	CDiskAddress::AbsSectorNumberToDiskAddress(dwAbsStartSectorNumber, daStartAddr, geoMetry);
}

void CDiskPartitionTable::SetEndSector(DWORD dwSectorNumber, DISK_GEOMETRY &geoMetry)
{
	dwAbsStartSectorNumber = CDiskAddress::SectorCylinderAlignmentLow(dwSectorNumber, geoMetry);
	CDiskAddress::AbsSectorNumberToDiskAddress(dwAbsStartSectorNumber, daEndAddr, geoMetry);
}

void CDiskPartitionTable::SetPartitionSize(DWORD dwSectorCount, DISK_GEOMETRY &geoMetry)
{
	dwTotalSectorCount = dwSectorCount;
	CDiskAddress::AbsSectorNumberToDiskAddress(dwAbsStartSectorNumber + dwTotalSectorCount - 1, daEndAddr, geoMetry);
}

void CDiskPartitionTable::SetPartitionSizeAlignmentHight(DWORD dwSectorCount, DISK_GEOMETRY &geoMetry)
{
	DWORD dwSectorPerCylinder;

	dwSectorPerCylinder = geoMetry.SectorsPerTrack * geoMetry.TracksPerCylinder;
	SetPartitionSize((dwSectorCount + dwSectorPerCylinder) / dwSectorPerCylinder * dwSectorPerCylinder -
		dwAbsStartSectorNumber % dwSectorPerCylinder, geoMetry);
}

void CDiskPartitionTable::SetPartitionSizeAlignmentLow(DWORD dwSectorCount, DISK_GEOMETRY &geoMetry)
{
	DWORD dwSectorPerCylinder;

	dwSectorPerCylinder = geoMetry.TracksPerCylinder * geoMetry.SectorsPerTrack;
	SetPartitionSize(dwSectorCount / dwSectorPerCylinder * dwSectorPerCylinder -
		dwAbsStartSectorNumber % dwSectorPerCylinder, geoMetry);
}

// ==========================================================================================

void CMasterBootSector::ToString(CString &strString)
{
	CString strTemp;

	strString.Empty();

	for (int i = 0; i < 4; i++)
	{
		dptsDiskPartTables[i].ToString(strTemp);
		strString += strTemp + "\n";
	}

	strTemp.Format("MagicNumber = 0x%04x", wMagicNumber);
	strString += strTemp;
}

// ==========================================================================================

CCavanDevice::CCavanDevice()
{

}

CCavanDevice::~CCavanDevice()
{

}

UINT CCavanDevice::GetDriveTypeFormat(const char *pbytFormat, ...)
{
	va_list ap;
	char bytsBuffer[1024];

	va_start(ap, pbytFormat);
	vsprintf(bytsBuffer, pbytFormat, ap);
	va_end(ap);

	return GetDriveType(bytsBuffer);
}

BOOL CCavanDevice::OpenPhysicalDrive(int iIndex, UINT nOpenFlags)
{
	return CCavanFile::OpenFormatFile(nOpenFlags, DISK_DEVICE_PREFIX "%d", iIndex);
}

BOOL CCavanDevice::OpenPatition(char cIndex, UINT nOpenFlags)
{
	return CCavanFile::OpenFormatFile(nOpenFlags, PARTITION_DEVICE_PREFIX "%c:", cIndex);
}

BOOL CCavanDevice::GetDeviceGeoMery(DISK_GEOMETRY &geoMetry)
{
	return DeviceIoControlSimple(IOCTL_DISK_GET_DRIVE_GEOMETRY, &geoMetry, sizeof(geoMetry));
}

BOOL CCavanDevice::GetDeviceCapability(ULONGLONG &ullCapability)
{
	DISK_GEOMETRY geoMetry;

	if (GetDeviceGeoMery(geoMetry) == false)
	{
		return false;
	}

	ullCapability = geoMetry.BytesPerSector * geoMetry.SectorsPerTrack *
		geoMetry.TracksPerCylinder * geoMetry.Cylinders.QuadPart;

	return true;
}

BOOL CCavanDevice::GetDeviceCapability(CString &strCapability)
{
	ULONGLONG ullCapability;

	if (GetDeviceCapability(ullCapability) == false)
	{
		return false;
	}

	CCavanText::SizeToText(ullCapability, strCapability);

	return true;
}

BOOL CCavanDevice::ReadMasterBootSector(CMasterBootSector &mbs)
{
	try
	{
		Seek(0, CFile::begin);
		Read(&mbs, sizeof(mbs));
	}
	catch (...)
	{
		return false;
	}

	return true;
}

BOOL CCavanDevice::WriteMasterBootSector(CMasterBootSector &mbs)
{
	try
	{
		Seek(0, CFile::begin);
		Write(&mbs, sizeof(mbs));
	}
	catch (...)
	{
		return false;
	}

	return true;
}

BOOL CCavanDevice::DeviceIoControlSimple(DWORD dwIoControlCode, LPVOID lpOutBuffer, DWORD nOutBufferSize)
{
	return ::DeviceIoControl((HANDLE)m_hFile, dwIoControlCode, NULL, 0, lpOutBuffer, nOutBufferSize, &m_dwIoCtrlBytesReturned, NULL);
}

DWORD CCavanDevice::GetIoCtrlByteReturned(void)
{
	return m_dwIoCtrlBytesReturned;
}

BOOL CCavanDevice::GetDeviceLayout(void *vpBuffer, DWORD dwBufferSize)
{
	return DeviceIoControlSimple(IOCTL_DISK_GET_DRIVE_LAYOUT, vpBuffer, dwBufferSize);
}

BOOL CCavanDevice::GetDeviceLayout(int iDevIndex, void *vpBuffer, DWORD dwBufferSize, DWORD &dwRetSize)
{
	CCavanDevice cvnDevice;

	if (cvnDevice.OpenPhysicalDrive(iDevIndex, 0) == false)
	{
		return false;
	}

	BOOL bRet;

	bRet = cvnDevice.GetDeviceLayout(vpBuffer, dwBufferSize);
	dwRetSize = cvnDevice.m_dwIoCtrlBytesReturned;

	cvnDevice.Close();

	return bRet;
}

BOOL CCavanDevice::GetPartitionInfo(PARTITION_INFORMATION *pPartInfo)
{
	return DeviceIoControlSimple(IOCTL_DISK_GET_PARTITION_INFO, pPartInfo, sizeof(PARTITION_INFORMATION));
}

BOOL CCavanDevice::GetPartitionInfo(char cDevIndex, PARTITION_INFORMATION *pPartInfo)
{
	CCavanDevice cvnDevice;

	if (cvnDevice.OpenPatition(cDevIndex, 0) == false)
	{
		return false;
	}

	BOOL bRet;

	bRet = cvnDevice.GetPartitionInfo(pPartInfo);

	cvnDevice.Close();

	return bRet;
}

BOOL CCavanDevice::UpdateDiskProperties(void)
{
	return DeviceIoControlSimple(IOCTL_DISK_UPDATE_PROPERTIES, NULL, 0);
}

BOOL CCavanDevice::UpdateDiskProperties(int iDevIndex)
{
	CCavanDevice cvnDevice;

	if (cvnDevice.OpenPhysicalDrive(iDevIndex, 0) == false)
	{
		return false;
	}

	BOOL bRet;

	bRet = cvnDevice.UpdateDiskProperties();

	cvnDevice.Close();

	return bRet;
}

