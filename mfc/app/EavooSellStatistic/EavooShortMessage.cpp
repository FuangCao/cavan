#include "stdafx.h"
#include "EavooSellStatistic.h"
#include "EavooShortMessage.h"

CEavooShortMessage::CEavooShortMessage(void)
{
	mSocket = INVALID_SOCKET;
}

CEavooShortMessage::~CEavooShortMessage(void)
{
	Uninitialize();
}

int CEavooShortMessage::Read(char *buff, UINT size, bool prompt)
{
	if (mFile.Read(buff, size) == size)
	{
		return size;
	}

	if (prompt)
	{
		AfxMessageBox("读数据失败");
	}

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
		return -1;
	}

	return 0;
}

int CEavooShortMessage::Receive(char *buff, int size, bool prompt)
{
	if (recv(mSocket, buff, size, 0) == size)
	{
		return size;
	}

	if (prompt)
	{
		AfxMessageBox("接收数据失败");
	}

	return -1;
}

int CEavooShortMessage::Send(const char *buff, int size)
{
	if (send(mSocket, buff, size, 0) == size)
	{
		return size;
	}

	AfxMessageBox("发送数据失败");

	return -1;
}

bool CEavooShortMessage::Initialize(const char *pathname, UINT flags, USHORT port, const char *ip)
{
	if (pathname)
	{
		if (mFile.Open(pathname, flags, NULL) == false)
		{
			AfxMessageBox("打开文件失败");
			return false;
		}

		if ((flags & (CFile::modeWrite | CFile::modeReadWrite)) && (flags & CFile::modeNoTruncate))
		{
			mFile.SeekToEnd();
		}
	}

	if (port && AdbServerConnect(port, ip) == false)
	{
		Uninitialize();
		return false;
	}

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
		if (Receive(&type, 1, false) < 0)
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

		case SMS_TYPE_PHONE:
			ret = ReceiveText(mPhone);
			if (ret < 0)
			{
				return false;
			}

			if (WriteText(type, mPhone, ret) < 0)
			{
				return false;
			}
			break;

		case SMS_TYPE_CONTENT:
			ret = ReceiveText(mContent);
			if (ret < 0)
			{
				return false;
			}

			if (WriteText(type, mContent, ret) < 0)
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
	list.InsertItem(0, mPhone);
	COleDateTime time(mDate);
	list.SetItemText(0, 1, time.Format("%Y年%m月%d日 %H时%M分%S秒"));
	list.SetItemText(0, 2, mContent);
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

	if (WriteText(SMS_TYPE_PHONE, mPhone) < 0)
	{
		return false;
	}

	if (WriteText(SMS_TYPE_CONTENT, mContent) < 0)
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

int CEavooShortMessage::ReadText(char *buff)
{
	int length;

	if (Read((char *)&length, sizeof(length)) < 0)
	{
		return -1;
	}

	if (Read(buff, length) < 0)
	{
		return -1;
	}

	buff[length] = 0;

	return length;
}

bool CEavooShortMessage::ReadFromFile(void)
{
	char type;

	mDate = 0;

	while (1)
	{
		if (Read(&type, 1, false) < 0)
		{
			return false;
		}

		switch (type)
		{
		case SMS_TYPE_DATE:
			if (ReadValue((char *)&mDate, sizeof(mDate)) < 0)
			{
				return false;
			}
			break;

		case SMS_TYPE_PHONE:
			if (ReadText(mPhone) < 0)
			{
				return false;
			}
			break;

		case SMS_TYPE_CONTENT:
			if (ReadText(mContent) < 0)
			{
				return false;
			}
			break;

		case SMS_TYPE_END:
			return true;

		default:
			AfxMessageBox("不能识别的类型");
			return false;
		}
	}

	return false;
}

bool CEavooShortMessage::AdbServerConnect(UINT port, const char *ip)
{
	if (WSAStartup(MAKEWORD(2, 2), &mWSAData) != 0)
	{
		AfxMessageBox("启动协议失败");
		return false;
	}

	mSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (mSocket == INVALID_SOCKET)
	{
		AfxMessageBox("分配套接字失败");
		return false;
	}

	if (AdbLocalConnect() == false)
	{
		AfxMessageBox("建立连接失败，请确认IP地址是否正确");
		closesocket(mSocket);
		return false;
	}

	char command[16];
	sprintf(command, "tcp:%04d", port);
	if (AdbSendCommand(command) == false)
	{
		AfxMessageBox("建立连接失败，请确认端口号是否正确");
		closesocket(mSocket);
		return false;
	}

	return true;
}

bool CEavooShortMessage::AdbLocalConnect(const char *ip)
{
	int ret;
	sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = inet_addr(ip);
	addr.sin_port = htons(ADB_SERVICE_PORT);

	ret = connect(mSocket, (sockaddr *)&addr, sizeof(addr));
	if (ret == 0)
	{
		return true;
	}

	ret = system("adb start-server");
	if (ret != 0)
	{
		AfxMessageBox("请安装ADB命令");
		return false;
	}

	return connect(mSocket, (sockaddr *)&addr, sizeof(addr)) == 0;
}

bool CEavooShortMessage::AdbReadStatus(void)
{
	if (Receive(mAdbStatus, 4) < 0)
	{
		AfxMessageBox("接收状态信息失败");
		return false;
	}

	if (strncmp(ADB_STATE_OKAY, mAdbStatus, 4) == 0)
	{
		return true;
	}

	if (strncmp(ADB_STATE_FAIL, mAdbStatus, 4) == 0)
	{
		AfxMessageBox("ADB协议错误(FAIL)");
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
		AfxMessageBox("接收状态信息失败");
	}
	else
	{
		mAdbStatus[length] = 0;
		AfxMessageBox(mAdbStatus);
	}

	return false;
}

bool CEavooShortMessage::AdbSendText(const char *text)
{
	int length = strlen(text);
	char buff[32];

	sprintf(buff, "%04x", length);
	if (Send(buff, 4) < 0)
	{
		AfxMessageBox("发送命令长度失败");
		return false;
	}

	if (Send(text, length) < 0)
	{
		AfxMessageBox("发送命令长度失败");
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
