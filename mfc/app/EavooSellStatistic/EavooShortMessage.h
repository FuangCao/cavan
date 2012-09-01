#if !defined(AFX_EAVOOSHORTMESSAGE_H__0481FD35_EB2A_4F18_AC53_F5EF9D1976D9__INCLUDED_)
#define AFX_EAVOOSHORTMESSAGE_H__0481FD35_EB2A_4F18_AC53_F5EF9D1976D9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EavooShortMessage.h : header file
//

#define MAX_LIST_SIZE		1000
#define RETRY_DELAY_TIME	1000
#define DEFAULT_SERVER_IP	"127.0.0.1"
#define DEFAULT_SERVER_PORT	8888
#define DEFAULT_CACHE_FILENAME			"eavoo_sell.dat"
#define DEFAULT_STATISTIC_FILENAME		"eavoo_statistic.dat"

#define ADB_SERVICE_PORT	5037
#define ADB_STATE_OKAY		"OKAY"
#define ADB_STATE_FAIL		"FAIL"

#define PROGRESS_MIN_COUNT	0xFF

#define NELEM(a) \
	(sizeof(a) / sizeof((a)[0]))

#define IS_UPPERCASE(a) \
	((a) >= 'A' && (a) <= 'Z')

#define IS_LOWERCASE(a) \
	((a) >= 'a' && (a) <= 'z')

#define IS_NUMBER(a) \
	((a) >= '0' && (a) <= '9')

enum
{
	SMS_TYPE_END = 0x00,
	SMS_TYPE_TEST,
	SMS_TYPE_ACK,
	SMS_TYPE_OKAY,
	SMS_TYPE_FAILED,
	SMS_TYPE_DATE,
	SMS_TYPE_ADDRESS,
	SMS_TYPE_BODY
};

class CEavooShortMessageBody
{
public:
	char mIMEI[32];
	char mSoftwareVersion[64];

public:
	CEavooShortMessageBody(const char *text = NULL);
	static int TextCmpLH(const char *left, const char *right);
	char *GetIMEI(void) {return mIMEI; };
	char *GetSoftwareVersion(void) {return mSoftwareVersion; };
	char *GetOneSep(const char *text, char *buff);
	bool ParseText(const char *text);
	char *GetProjectName(char *buff);
};

class CEavooShortMessage
{
public:
	time_t mDate;
	char mAddress[24];
	char mBody[512];

public:
	bool InsertIntoList(CListCtrl &list);
	void Initialize(void);
	bool IsInvalid(void);
	bool IsValid(void);
	int ToTxtLine(char *buff, const char *prefix = "", const char *sufix = "");
	int ToXmlLine(char *buff, const char *prefix = "", const char *sufix = "");
};

class CEavooShortMessageHelper
{
private:
	CEavooShortMessage mShortMessage;

private:
	SOCKET mSocket;
	CFile mFile;
	WSAData mWSAData;
	char mAdbStatus[512];
	char mIpAddress[32];
	UINT mPort;
	CProgressCtrl &mProgress;
	CStatic &mState;

public:
	CEavooShortMessageHelper(CProgressCtrl &progress, CStatic &state);
	~CEavooShortMessageHelper(void);

	bool Initialize(UINT flags, const char *filename = NULL, UINT port = 0, const char *ip = DEFAULT_SERVER_IP);
	void Uninitialize(void);

	time_t *GetDate(void)
	{
		return &mShortMessage.mDate;
	}

	int Flush(void);
	int Receive(char *buff, int size);
	int Send(const char *buff, int size);

	DWORD WriteToFile(void);
	DWORD ReadFromFile(void);
	DWORD ReadFromFileOld(void);

	DWORD GetFileLength(void)
	{
		return mFile.GetLength();
	}

	void SeekToBegin(void)
	{
		mFile.SeekToBegin();
	}

	DWORD Seek(LONG lOff)
	{
		return ::SetFilePointer((HANDLE)mFile.m_hFile, lOff, NULL, FILE_BEGIN);
	}

	const char *GetMessageBody(void)
	{
		return mShortMessage.mBody;
	}

	CEavooShortMessage &GetShortMessage(void)
	{
		return mShortMessage;
	}

	int ReceiveValue(char *buff, int size);
	int ReceiveText(char *buff);
	bool SendResponse(char type);
	bool ReceiveFromNetwork(void);

	bool AdbServerConnect(void);
	bool AdbLocalConnect(void);
	bool AdbReadStatus(void);
	bool AdbSendText(const char *text);
	bool AdbSendCommand(const char *command);

	bool ParseBody(CEavooShortMessageBody &body);

	static int WriteTextToFile(CFile &file, const char *buff, int length);
	bool ExportXmlFile(CFile &file);
	bool ExportTxtFile(CFile &file);
	bool ImportDatabase(CEavooShortMessageHelper &helper);

	void ShowShortMessage(void);
};

#endif // !defined(AFX_EAVOOSHORTMESSAGE_H__0481FD35_EB2A_4F18_AC53_F5EF9D1976D9__INCLUDED_)
