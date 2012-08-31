// EavooShortMessage.cpp : implementation file
//

#include "stdafx.h"
#include "EavooSellStatistic.h"
#include "EavooShortMessage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CEavooShortMessageBody::CEavooShortMessageBody(const char *text)
{
	if (text)
	{
		ParseText(text);
	}
}

int CEavooShortMessageBody::TextCmpLH(const char *left, const char *right)
{
	while (*left)
	{
		if (*left != *right)
		{
			return *left - *right;
		}

		left++;
		right++;
	}

	return 0;
}

char *CEavooShortMessageBody::GetOneSep(const char *text, char *buff)
{
	const char *p, *p_end;

	for (p_end = text; *p_end && *p_end != ','; p_end++);

	for (p = text; *p == ' ' && p < p_end; p++);

	while (p < p_end)
	{
		*buff++ = *p++;
	}

	*buff = 0;

	return (char *)p;
}

bool CEavooShortMessageBody::ParseText(const char *text)
{
	if (TextCmpLH("EDUA#", text))
	{
		return false;
	}

	text += 5;

	char *seps[] =
	{
		mIMEI, mSoftwareVersion
	};

	for (int i = 0; i < NELEM(seps); i++)
	{
		text = GetOneSep(text, seps[i]);
		if (*text)
		{
			text++;
		}
		else
		{
			return false;
		}
	}

	return true;
}

char *CEavooShortMessageBody::GetProjectName(char *buff)
{
	const char *p, *q;
	char *buff_bak;

	for (p = mSoftwareVersion; ; p++)
	{
		if (p[0] == '.' && p[1] == 'E' && IS_UPPERCASE(p[2]) && IS_UPPERCASE(p[-1]))
		{
			break;
		}

		if (*p == 0)
		{
			return NULL;
		}
	}

	p += 2;

	for (q = p; ; q++)
	{
		if (q[0] == '.' && q[1] == 'V' && IS_NUMBER(q[2]))
		{
			break;
		}

		if (*q == 0)
		{
			q = p + 8;
			break;
		}
	}

	for (q -= 4, buff_bak = buff; p < q; p++, buff++)
	{
		*buff = *p;
	}

	if (buff == buff_bak)
	{
		return NULL;
	}

	*buff = 0;

	return buff_bak;
}

// ============================================================

bool CEavooShortMessage::InsertIntoList(CListCtrl &list)
{
	int index = list.InsertItem(0, mAddress);
	if (index < 0)
	{
		return false;
	}

	COleDateTime time(mDate);
	list.SetItemText(index, 1, time.Format("%Y-%m-%d %H:%M:%S"));
	list.SetItemText(index, 2, mBody);

	int count = list.GetItemCount();
	if (count > MAX_LIST_SIZE)
	{
		list.DeleteItem(count - 1);
	}

	return true;
}

void CEavooShortMessage::Initialize(void)
{
	mDate = 0;
	mAddress[0] = 0;
	mBody[0] = 0;
}

bool CEavooShortMessage::IsInvalid(void)
{
	return mDate == 0 || mAddress[0] == 0 || mBody[0] == 0;
}

bool CEavooShortMessage::IsValid(void)
{
	return mDate && mAddress[0] && mBody[0];
}

int CEavooShortMessage::ToTextLine(char *buff, const char *prefix, const char *sufix)
{
	COleDateTime time(mDate);
	return sprintf(buff, "%s%s\t%s\t%s%s", prefix, mAddress, time.Format("%Y-%m-%d %H:%M:%S"), mBody, sufix);
}

int CEavooShortMessage::ToXmlLine(char *buff, const char *prefix, const char *sufix)
{
	COleDateTime time(mDate);
	return sprintf(buff, "%s<message date=\"%s\" address=\"%s\" body=\"%s\" />%s", prefix, time.Format("%Y-%m-%d %H:%M:%S"), mAddress, mBody, sufix);
}

// ============================================================

CEavooShortMessageHelper::CEavooShortMessageHelper(void)
{
	mSocket = INVALID_SOCKET;
}

CEavooShortMessageHelper::~CEavooShortMessageHelper(void)
{
	Uninitialize();
}

int CEavooShortMessageHelper::Receive(char *buff, int size)
{
	if (recv(mSocket, buff, size, 0) == size)
	{
		return size;
	}

	closesocket(mSocket);
	mSocket = INVALID_SOCKET;

	return -1;
}

int CEavooShortMessageHelper::Send(const char *buff, int size)
{
	if (send(mSocket, buff, size, 0) == size)
	{
		return size;
	}

	// AfxMessageBox("发送数据失败");
	closesocket(mSocket);
	mSocket = INVALID_SOCKET;

	return -1;
}

bool CEavooShortMessageHelper::Initialize(UINT flags, const char *pathname, UINT port, const char *ip)
{
	if (WSAStartup(MAKEWORD(2, 2), &mWSAData) != 0)
	{
		AfxMessageBox("启动网络协议失败");
		return false;
	}

	if (theApp.OpenDatabase(mFile, pathname, flags) == false)
	{
		return false;
	}

	strcpy(mIpAddress, ip);
	mPort = port;

	return true;
}

void CEavooShortMessageHelper::Uninitialize(void)
{
	if (mFile.m_hFile != CFile::hFileNull)
	{
		mFile.Close();
		mFile.m_hFile = CFile::hFileNull;
	}

	if (mSocket != INVALID_SOCKET)
	{
		shutdown(mSocket, 0);
		closesocket(mSocket);
		mSocket = INVALID_SOCKET;
	}
}

int CEavooShortMessageHelper::ReceiveValue(char *buff, int size)
{
	return Receive(buff, size);
}

int CEavooShortMessageHelper::ReceiveText(char *buff)
{
	int length;

	if (Receive((char *)&length, sizeof(length)) < 0 || Receive(buff, length) < 0)
	{
		return -1;
	}

	buff[length] = 0;

	return length;
}

bool CEavooShortMessageHelper::SendResponse(char type)
{
	return Send(&type, 1) == 1;
}

bool CEavooShortMessageHelper::ReceiveFromNetwork(void)
{
	char type;
	int ret;

	mShortMessage.Initialize();

	while (1)
	{
		if (Receive(&type, 1) < 0)
		{
			return false;
		}

		switch (type)
		{
		case SMS_TYPE_DATE:
			if (ReceiveValue((char *)&mShortMessage.mDate, sizeof(mShortMessage.mDate)) < 0)
			{
				return false;
			}
			break;

		case SMS_TYPE_ADDRESS:
			ret = ReceiveText(mShortMessage.mAddress);
			if (ret < 0)
			{
				return false;
			}
			break;

		case SMS_TYPE_BODY:
			ret = ReceiveText(mShortMessage.mBody);
			if (ret < 0)
			{
				return false;
			}
			break;

		case SMS_TYPE_TEST:
			if (SendResponse(SMS_TYPE_ACK) == false)
			{
				return false;
			}
			continue;

		case SMS_TYPE_END:
			if (mShortMessage.IsInvalid())
			{
				SendResponse(SMS_TYPE_FAILED);
				return false;
			}
			return SendResponse(SMS_TYPE_ACK);

		default:
			return false;
		}
	}

	return false;
}

DWORD CEavooShortMessageHelper::WriteToFile(void)
{
	return theApp.WriteDatabase(mFile, (const char *)&mShortMessage, sizeof(mShortMessage));
}

DWORD CEavooShortMessageHelper::ReadFromFile(void)
{
	DWORD rdLength;

	mShortMessage.Initialize();
	rdLength = theApp.ReadDatabase(mFile, (char *)&mShortMessage, sizeof(mShortMessage));
	if (rdLength == 0 || mShortMessage.IsInvalid())
	{
		return 0;
	}

	return rdLength;
}

DWORD CEavooShortMessageHelper::ReadFromFileOld(void)
{
	return theApp.ReadDatabaseOld(mFile, mShortMessage);
}

bool CEavooShortMessageHelper::AdbServerConnect()
{
	char command[16];
	sprintf(command, "tcp:%04d", mPort);

	return AdbSendCommand(command);
}

bool CEavooShortMessageHelper::AdbLocalConnect()
{
	int ret;
	sockaddr_in addr;

	if (mSocket != INVALID_SOCKET)
	{
		closesocket(mSocket);
		mSocket = INVALID_SOCKET;
	}

	mSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (mSocket == INVALID_SOCKET)
	{
		// AfxMessageBox("分配套接字失败");
		return false;
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = inet_addr(mIpAddress);
	addr.sin_port = htons(ADB_SERVICE_PORT);

	ret = connect(mSocket, (sockaddr *)&addr, sizeof(addr));
	if (ret == 0)
	{
		return true;
	}

	system("adb kill-server");
	ret = system("adb start-server");
	if (ret != 0)
	{
		AfxMessageBox("请安装ADB命令");
		closesocket(mSocket);
		mSocket = INVALID_SOCKET;
		return false;
	}

	Sleep(2000);

	ret = connect(mSocket, (sockaddr *)&addr, sizeof(addr));
	if (ret == 0)
	{
		return true;
	}

	// AfxMessageBox("建立连接失败\n请确认IP地址是否正确");
	closesocket(mSocket);
	mSocket = INVALID_SOCKET;

	return false;
}

bool CEavooShortMessageHelper::AdbReadStatus(void)
{
	if (Receive(mAdbStatus, 4) < 0)
	{
		// AfxMessageBox("接收状态信息失败");
		return false;
	}

	if (strncmp(ADB_STATE_OKAY, mAdbStatus, 4) == 0)
	{
		return true;
	}

	if (strncmp(ADB_STATE_FAIL, mAdbStatus, 4) == 0)
	{
		// AfxMessageBox("ADB协议错误(FAIL)");
		return false;
	}

	int length;

	sscanf(mAdbStatus, "%04x", &length);
	if (length > sizeof(mAdbStatus) - 1)
	{
		length = sizeof(mAdbStatus) - 1;
	}

	if (Receive(mAdbStatus, length) < 0)
	{
		// AfxMessageBox("接收状态信息失败");
		return false;
	}

	mAdbStatus[length] = 0;
	// AfxMessageBox(mAdbStatus);

	return false;
}

bool CEavooShortMessageHelper::AdbSendText(const char *text)
{
	int length = strlen(text);
	char buff[32];

	sprintf(buff, "%04x", length);
	if (Send(buff, 4) < 0)
	{
		// AfxMessageBox("发送命令长度失败");
		return false;
	}

	if (Send(text, length) < 0)
	{
		// AfxMessageBox("发送命令长度失败");
		return false;
	}

	return AdbReadStatus();
}

bool CEavooShortMessageHelper::AdbSendCommand(const char *command)
{
	if (strncmp("host", command, 4))
	{
		if (AdbSendText("host:transport-any") == false)
		{
			return false;
		}
	}

	return AdbSendText(command);
}

bool CEavooShortMessageHelper::ParseBody(CEavooShortMessageBody &body)
{
	return body.ParseText(mShortMessage.mBody);
}

void CEavooShortMessageHelper::ShowShortMessage(void)
{
	char buff[1024];

	mShortMessage.ToXmlLine(buff);
	AfxMessageBox(buff);
}

int CEavooShortMessageHelper::WriteTextToFile(CFile &file, const char *buff, int length)
{
	if (length < 0)
	{
		length = strlen(buff);
	}

	DWORD dwWrite;
	if (::WriteFile((HANDLE)file.m_hFile, buff, length, &dwWrite, NULL) == false || ((DWORD)length) != dwWrite)
	{
		return -1;
	}

	return length;
}

bool CEavooShortMessageHelper::ExportXmlFile(CFile &file, CProgressCtrl &progress, CStatic &state)
{
	char buff[1024];
	DWORD totalLength, rdLength;
	double rdTotal, percent;
	int wrLength;
	unsigned char count;

	WriteTextToFile(file, "<?xml version=\"1.0\" encoding=\"ascii\" ?>\r\n", -1);
	WriteTextToFile(file, "<messages>\r\n", -1);

	totalLength = mFile.GetLength();
	rdTotal = 0;
	progress.SetRange(0, 100);

	for (count = 0; ; count++)
	{
		rdLength = ReadFromFile();
		if ((count & PROGRESS_MIN_COUNT) == 0 || rdLength == 0)
		{
			percent = rdTotal * 100 / totalLength;
			progress.SetPos((int)(percent));
			sprintf(buff, "%0.2f%%", percent);
			state.SetWindowText(buff);

			if (rdLength == 0)
			{
				break;
			}
		}

		wrLength = mShortMessage.ToXmlLine(buff, "\t", "\r\n");
		WriteTextToFile(file, buff, wrLength);

		rdTotal += rdLength;
	}

	WriteTextToFile(file, "</messages>", -1);

	return rdTotal == totalLength;
}

bool CEavooShortMessageHelper::ExportTextFile(CFile &file, CProgressCtrl &progress, CStatic &state)
{
	char buff[1024];
	DWORD totalLength, rdLength;
	double rdTotal, percent;
	int wrLength;
	unsigned char count;

	totalLength = mFile.GetLength();
	rdTotal = 0;
	progress.SetRange(0, 100);

	for (count = 0; ; count++)
	{
		rdLength = ReadFromFile();
		if ((count & PROGRESS_MIN_COUNT) == 0 || rdLength == 0)
		{
			percent = rdTotal * 100 / totalLength;
			progress.SetPos((int)(percent));
			sprintf(buff, "%0.2lf%%", percent);
			state.SetWindowText(buff);

			if (rdLength == 0)
			{
				break;
			}
		}

		wrLength = mShortMessage.ToTextLine(buff, "", "\r\n");
		file.Write(buff, wrLength);

		rdTotal += rdLength;
	}

	return rdTotal == totalLength;
}

bool CEavooShortMessageHelper::ImportDatabase(CEavooShortMessageHelper &helper, CProgressCtrl &progress, CStatic &state)
{
	CEavooShortMessage &message = helper.GetShortMessage();
	DWORD (CEavooShortMessageHelper::*ReadHandler)(void);

	if (helper.ReadFromFileOld() == 0)
	{
		ReadHandler = helper.ReadFromFile;
	}
	else
	{
		ReadHandler = helper.ReadFromFileOld;
	}

	helper.SeekToBegin();
	DWORD totalLength, rdLength;
	double rdTotal, percent;
	char buff[8];
	unsigned char count;

	totalLength = helper.GetFileLength();
	rdTotal = 0;
	progress.SetRange(0, 100);

	for (count = 0; ; count++)
	{
		rdLength = (helper.*ReadHandler)();
		if ((count & PROGRESS_MIN_COUNT) == 0 || rdLength == 0)
		{
			percent = rdTotal * 100 / totalLength;
			progress.SetPos((int)percent);
			sprintf(buff, "%0.2lf%%", percent);
			state.SetWindowText(buff);

			if (rdLength == 0)
			{
				break;
			}
		}

		if (theApp.WriteDatabaseNoSync(mFile, (const char *)&message, sizeof(message)) == 0)
		{
			return false;
		}

		rdTotal += rdLength;
	}

	return rdTotal == totalLength;
}
