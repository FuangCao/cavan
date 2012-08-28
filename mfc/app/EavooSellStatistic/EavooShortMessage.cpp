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

bool CEavooShortMessageHelper::Initialize(const char *pathname, UINT flags, UINT port, const char *ip)
{
	if (WSAStartup(MAKEWORD(2, 2), &mWSAData) != 0)
	{
		AfxMessageBox("启动网络协议失败");
		return false;
	}

	if (pathname)
	{
		if (mFile.Open(pathname, flags, NULL) == false)
		{
			// AfxMessageBox("打开文件失败");
			return false;
		}

		if ((flags & (CFile::modeWrite | CFile::modeReadWrite)) && (flags & CFile::modeNoTruncate))
		{
			mFile.SeekToEnd();
		}
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
	DWORD dwWrite;
	if (::WriteFile((HANDLE)mFile.m_hFile, &mShortMessage, sizeof(mShortMessage), &dwWrite, NULL) == FALSE)
	{
		return 0;
	}

	if (::FlushFileBuffers((HANDLE)mFile.m_hFile))
	{
		return dwWrite;
	}

	return 0;
}

DWORD CEavooShortMessageHelper::ReadFromFile(void)
{
	DWORD dwRead;
	if (::ReadFile((HANDLE)mFile.m_hFile, &mShortMessage, sizeof(mShortMessage), &dwRead, NULL) && mShortMessage.IsValid())
	{
		return dwRead;
	}

	mFile.Close();
	return 0;
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

void CEavooShortMessageHelper::InsertIntoList(CListCtrl &list)
{
	mShortMessage.InsertIntoList(list);
}
