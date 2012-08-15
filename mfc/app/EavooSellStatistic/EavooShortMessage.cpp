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
			return NULL;
		}
	}

	for (q -= 4, buff_bak = buff; p < q; p++, buff++)
	{
		*buff = *p;
	}

	*buff = 0;

	return buff_bak;
}

// ============================================================

CEavooShortMessage::CEavooShortMessage(void)
{
	mSocket = INVALID_SOCKET;
}

CEavooShortMessage::~CEavooShortMessage(void)
{
	Uninitialize();
}

int CEavooShortMessage::Read(char *buff, UINT size)
{
	if (mFile.Read(buff, size) == size)
	{
		return size;
	}

	mFile.Close();
	mFile.m_hFile = CFile::hFileNull;

	return -1;
}

int CEavooShortMessage::Write(const char *buff, UINT size)
{
	try
	{
		mFile.Write(buff, size);
	}
	catch (...)
	{
		AfxMessageBox("写数据失败");
		mFile.Close();
		mFile.m_hFile = CFile::hFileNull;
		return -1;
	}

	return size;
}

int CEavooShortMessage::Flush(void)
{
	try
	{
		mFile.Flush();
	}
	catch (...)
	{
		AfxMessageBox("刷数据失败");
		mFile.Close();
		mFile.m_hFile = CFile::hFileNull;
		return -1;
	}

	return 0;
}

int CEavooShortMessage::Receive(char *buff, int size)
{
	if (recv(mSocket, buff, size, 0) == size)
	{
		return size;
	}

	closesocket(mSocket);
	mSocket = INVALID_SOCKET;

	return -1;
}

int CEavooShortMessage::Send(const char *buff, int size)
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

bool CEavooShortMessage::Initialize(const char *pathname, UINT flags, UINT port, const char *ip)
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

void CEavooShortMessage::Uninitialize(void)
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

int CEavooShortMessage::ReceiveValue(char *buff, int size)
{
	return Receive(buff, size);
}

int CEavooShortMessage::ReceiveText(char *buff)
{
	int length;

	if (Receive((char *)&length, sizeof(length)) < 0 || Receive(buff, length) < 0)
	{
		return -1;
	}

	buff[length] = 0;

	return length;
}

bool CEavooShortMessage::SendResponse(char type)
{
	return Send(&type, 1) == 1;
}

bool CEavooShortMessage::ReceiveFromNetwork(void)
{
	char type;
	int ret;

	while (1)
	{
		if (Receive(&type, 1) < 0)
		{
			return false;
		}

		switch (type)
		{
		case SMS_TYPE_DATE:
			if (ReceiveValue((char *)&mDate, sizeof(mDate)) < 0)
			{
				return false;
			}

			if (WriteValue(type, (char *)&mDate, sizeof(mDate)) < 0)
			{
				return false;
			}
			break;

		case SMS_TYPE_ADDRESS:
			ret = ReceiveText(mAddress);
			if (ret < 0)
			{
				return false;
			}

			if (WriteText(type, mAddress, ret) < 0)
			{
				return false;
			}
			break;

		case SMS_TYPE_BODY:
			ret = ReceiveText(mBody);
			if (ret < 0)
			{
				return false;
			}

			if (WriteText(type, mBody, ret) < 0)
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
			if (SendResponse(SMS_TYPE_ACK) == false)
			{
				return false;
			}

			if (WriteType(type) == false)
			{
				return false;
			}

			return Flush() == 0;

		default:
			return false;
		}
	}

	return false;
}

void CEavooShortMessage::InsertIntoList(CListCtrl &list)
{
	int count = list.GetItemCount();
	if (count > MAX_LIST_SIZE)
	{
		list.DeleteItem(count - 1);
	}

	list.InsertItem(0, mAddress);
	COleDateTime time(mDate);
	list.SetItemText(0, 1, time.Format("%Y年%m月%d日 %H时%M分%S秒"));
	list.SetItemText(0, 2, mBody);
}

bool CEavooShortMessage::WriteType(char type)
{
	return Write(&type, 1) == 1;
}

int CEavooShortMessage::WriteText(char type, const char *text)
{
	if (Write(&type, 1) < 0)
	{
		return -1;
	}

	int length = strlen(text);
	if (Write((char *)&length, sizeof(length)) < 0)
	{
		return -1;
	}

	if (Write(text, length) < 0)
	{
		return -1;
	}

	return length;
}

int CEavooShortMessage::WriteText(char type, const char *text, int length)
{
	if (Write(&type, 1) < 0)
	{
		return -1;
	}

	if (Write((char *)&length, sizeof(length)) < 0)
	{
		return -1;
	}

	if (Write(text, length) < 0)
	{
		return -1;
	}

	return length;
}

int CEavooShortMessage::WriteValue(char type, const char *data, int size)
{
	if (Write(&type, 1) < 0)
	{
		return -1;
	}

	if (Write(data, size) < 0)
	{
		return -1;
	}

	return size;
}

bool CEavooShortMessage::WriteToFile(void)
{
	if (WriteValue(SMS_TYPE_DATE, (char *)&mDate, sizeof(mDate)) < 0)
	{
		return false;
	}

	if (WriteText(SMS_TYPE_ADDRESS, mAddress) < 0)
	{
		return false;
	}

	if (WriteText(SMS_TYPE_BODY, mBody) < 0)
	{
		return false;
	}

	if (WriteType(SMS_TYPE_END) == false)
	{
		return false;
	}

	return Flush() == 0;
}

int CEavooShortMessage::ReadValue(char *buff, UINT size)
{
	return Read(buff, size);
}

int CEavooShortMessage::ReadText(char *buff, int size)
{
	int length;

	if (Read((char *)&length, sizeof(length)) < 0)
	{
		return -1;
	}

	if (length <= 0 || length >= size || Read(buff, length) < 0)
	{
		return -1;
	}

	buff[length] = 0;

	return length;
}

bool CEavooShortMessage::ReadFromFile(void)
{
	char type;

	mBody[0] = 0;
	mAddress[0] = 0;

	while (1)
	{
		if (Read(&type, 1) < 0)
		{
			return false;
		}

		switch (type)
		{
		case SMS_TYPE_DATE:
			if (ReadValue((char *)&mDate, sizeof(mDate)) < 0)
			{
				goto out_bad_database;
			}
			break;

		case SMS_TYPE_ADDRESS:
			if (ReadText(mAddress, sizeof(mAddress)) < 0)
			{
				goto out_bad_database;
			}
			break;

		case SMS_TYPE_BODY:
			if (ReadText(mBody, sizeof(mBody)) < 0)
			{
				goto out_bad_database;
			}
			break;

		case SMS_TYPE_END:
			if (mBody[0] == 0 || mAddress[0] == 0)
			{
				goto out_bad_database;
			}
			return true;

		default:
			return false;
		}
	}

out_bad_database:
	AfxMessageBox("数据库已损坏");
	return false;
}

bool CEavooShortMessage::AdbServerConnect()
{
	char command[16];
	sprintf(command, "tcp:%04d", mPort);

	return AdbSendCommand(command);
}

bool CEavooShortMessage::AdbLocalConnect()
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

bool CEavooShortMessage::AdbReadStatus(void)
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

bool CEavooShortMessage::AdbSendText(const char *text)
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

bool CEavooShortMessage::AdbSendCommand(const char *command)
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

bool CEavooShortMessage::ParseBody(CEavooShortMessageBody &body)
{
	return body.ParseText(mBody);
}
