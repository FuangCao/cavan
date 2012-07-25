#ifndef AFX_CEAVOOSHORTMESSAGE_H__3F41B8B2_168E_4DA3_99E0_94F704ED7CD6__INCLUDED_
#define AFX_CEAVOOSHORTMESSAGE_H__3F41B8B2_168E_4DA3_99E0_94F704ED7CD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define DEFAULT_SERVER_IP	"127.0.0.1"
#define DEFAULT_SERVER_PORT	8888
#define CACHE_FILENAME		"eavoo_sell.dat"

#define ADB_SERVICE_PORT	5037
#define ADB_STATE_OKAY		"OKAY"
#define ADB_STATE_FAIL		"FAIL"

#define NELEM(a) \
	(sizeof(a) / sizeof((a)[0]))

enum
{
	SMS_TYPE_END = 0x00,
	SMS_TYPE_TEST,
	SMS_TYPE_ACK,
	SMS_TYPE_OKAY,
	SMS_TYPE_FAILED,
	SMS_TYPE_DATE,
	SMS_TYPE_PHONE,
	SMS_TYPE_CONTENT
};

class CEavooShortMessage
{
private:
	time_t mDate;
	char mPhone[32];
	char mContent[1024];

private:
	SOCKET mSocket;
	CFile mFile;
	WSAData mWSAData;
	char mAdbStatus[512];

public:
	CEavooShortMessage(void);
	~CEavooShortMessage(void);

	bool Initialize(const char *pathname, UINT flags, USHORT port = 0, const char *ip = NULL);
	void Uninitialize(void);
	void InsertIntoList(CListCtrl &list);

	time_t *GetDate(void)
	{
		return &mDate;
	}

	int Read(char *buff, UINT size, bool prompt = true);
	int Write(const char *buff, UINT size);
	int Flush(void);
	int Receive(char *buff, int size, bool prompt = true);
	int Send(const char *buff, int size);

	int WriteText(char type, const char *text);
	int WriteText(char type, const char *text, int length);
	int WriteValue(char type, const char *data, int size);
	bool WriteType(char type);
	bool WriteToFile(void);

	int ReadValue(char *buff, UINT size);
	int ReadText(char *buff);
	bool ReadFromFile(void);

	int ReceiveValue(char *buff, int size);
	int ReceiveText(char *buff);
	bool SendResponse(char type);
	bool ReceiveFromNetwork(void);

	bool AdbServerConnect(UINT port = DEFAULT_SERVER_PORT, const char *ip = DEFAULT_SERVER_IP);
	bool AdbLocalConnect(const char *ip = DEFAULT_SERVER_IP);
	bool AdbReadStatus(void);
	bool AdbSendText(const char *text);
	bool AdbSendCommand(const char *command);
};

#endif
