// ProxyService.h: interface for the CProxyService class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROXYSERVICE_H__E6318F0D_4121_43CA_A7A5_38E7772171DB__INCLUDED_)
#define AFX_PROXYSERVICE_H__E6318F0D_4121_43CA_A7A5_38E7772171DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CavanService.h"
#include "ProxyServerDlg.h"

#define ADB_SERVICE_PORT1	5037
#define ADB_SERVICE_PORT2	5038

class CProxyThread;
class CProxyServerDlg;

enum CProxyProcotolType
{
	PROXY_PROTOCOL_TYPE_NONE,
	PROXY_PROTOCOL_TYPE_TCP,
	PROXY_PROTOCOL_TYPE_UDP,
	PROXY_PROTOCOL_TYPE_ADB
};

class CCavanTransport
{
private:
	bool mNeedClose;

protected:
	const CProxyProcotolType mType;
	SOCKET mSocket;
	struct sockaddr_in mSockAddr;
	int mSockAddrLen;

public:
	CCavanTransport(CProxyProcotolType type, SOCKET sockfd, struct sockaddr_in *addr, bool bNeedClose = true);
	virtual ~CCavanTransport()
	{
		Close();
	}

	virtual bool Open(WORD port, DWORD ip)
	{
		return false;
	}

	virtual void Close(void);

	virtual int ReceiveData(void *buff, int size) = 0;
	virtual int SendData(const void *buff, int size) = 0;

	virtual CCavanTransport *Accept(void *buff, int *size)
	{
		return NULL;
	}

	friend CProxyThread;
};

class CTransportTcpClient : public CCavanTransport
{
public:
	CTransportTcpClient(SOCKET sockfd = INVALID_SOCKET, struct sockaddr_in *addr = NULL, CProxyProcotolType type = PROXY_PROTOCOL_TYPE_TCP) : CCavanTransport(type, sockfd, addr) {}
	virtual bool Open(WORD port, DWORD ip = INADDR_LOOPBACK);

	virtual int ReceiveData(void *buff, int size)
	{
		return recv(mSocket, (char *)buff, size, 0);
	}

	virtual int SendData(const void *buff, int size)
	{
		return send(mSocket, (char *)buff, size, 0);
	}
};

class CTransportAdbClient : public CTransportTcpClient
{
private:
	char mStatus[1024];

public:
	CTransportAdbClient(SOCKET sockfd = INVALID_SOCKET, struct sockaddr_in *addr = NULL, CProxyProcotolType type = PROXY_PROTOCOL_TYPE_ADB) : CTransportTcpClient(sockfd, addr, type) {}
	virtual bool Open(WORD port = 8888, DWORD ip = INADDR_ANY);

	bool Connect(DWORD ip);
	bool CreateTcpLink(WORD port);
	bool SendText(const char *text);
	bool CheckStatus(void);
};

class CTransportTcpService : public CTransportTcpClient
{
public:
	virtual bool Open(WORD port, DWORD ip = INADDR_ANY);
	virtual CCavanTransport *Accept(void *buff = NULL, int *size = NULL);
};

class CTransportUdpClient : public CCavanTransport
{
public:
	CTransportUdpClient(SOCKET sockfd = INVALID_SOCKET, struct sockaddr_in *addr = NULL, bool bNeedClose = true) : CCavanTransport(PROXY_PROTOCOL_TYPE_UDP, sockfd, addr, bNeedClose) {}
	virtual bool Open(WORD port, DWORD ip = INADDR_LOOPBACK);

	virtual int ReceiveData(void *buff, int size)
	{
		return recvfrom(mSocket, (char *)buff, size, 0, (struct sockaddr *)&mSockAddr, &mSockAddrLen);
	}

	virtual int SendData(const void *buff, int size)
	{
		return sendto(mSocket, (char *)buff, size, 0, (struct sockaddr *)&mSockAddr, mSockAddrLen);
	}
};

class CTransportUdpService : public CTransportUdpClient
{
public:
	virtual bool Open(WORD port, DWORD ip = INADDR_ANY);
	virtual CCavanTransport *Accept(void *buff, int *size);
};

class CProxyThread : public CCavanThread
{
private:
	CMutex mLock;
	CCavanTransport *mServiceTransport;
	CCavanTransport *mClientTransport;
	CCavanTransport *mProxyTransport;
	WORD mProxyPort;
	DWORD mProxyIP;
	CProxyProcotolType mProxyProtocol;
	CListCtrl &mListCtrl;
	CProgressCtrl &mProgressCtrl;

protected:
	virtual bool Run(void);

public:
	CProxyThread(int nIndex, CListCtrl &list, CProgressCtrl &progress) :
		CCavanThread(nIndex), mListCtrl(list), mProgressCtrl(progress), mProxyTransport(NULL), mClientTransport(NULL) {}

	virtual ~CProxyThread(void)
	{
		Stop();

		if (mProxyTransport)
		{
			delete mProxyTransport;
		}
	}

	virtual void Prepare(CCavanTransport *trspService, WORD wProxyPort = 8888, CProxyProcotolType nProxyProtocol = PROXY_PROTOCOL_TYPE_ADB, DWORD dwProxyIP = INADDR_LOOPBACK);
	virtual bool Start(void);
	virtual void Stop(void);

	void SetStatusText(const char *text)
	{
		mListCtrl.SetItemText(mIndex, 1, text);
	}

	void SetIpText(const char *text)
	{
		mListCtrl.SetItemText(mIndex, 2, text);
	}

	void SetPortText(const char *text)
	{
		mListCtrl.SetItemText(mIndex, 3, text);
	}

	void SetPortText(WORD port)
	{
		char buff[16];

		sprintf(buff, "%04d", port);
		SetPortText(buff);
	}
};

class CProxyService : public CCavanService
{
private:
	CMutex mLock;
	WORD mLocalPort;
	WORD mProxyPort;
	DWORD mProxyIP;
	CProxyProcotolType mLocalProtocol;
	CProxyProcotolType mProxyProtocol;
	CCavanTransport *mServiceTransport;
	CProxyServerDlg *mProxyDialog;

public:
	CProxyService(CProxyServerDlg *dlg) :
		CCavanService(), mProxyDialog(dlg), mLocalPort(8888), mProxyPort(8888), mProxyIP(INADDR_LOOPBACK),
		mLocalProtocol(PROXY_PROTOCOL_TYPE_TCP), mProxyProtocol(PROXY_PROTOCOL_TYPE_ADB), mServiceTransport(NULL) {}

	virtual ~CProxyService()
	{
		Stop();
		DestoryThreads();
	}

	void Prepare(WORD wProxyPort, WORD wLocalPort = 8888, DWORD dwProxyIP = INADDR_LOOPBACK, CProxyProcotolType nLocalProtocol = PROXY_PROTOCOL_TYPE_TCP, CProxyProcotolType nProxyProtocol = PROXY_PROTOCOL_TYPE_ADB, int nDaemonCount = 20);
	bool CreateThreads(void);
	void DestoryThreads(void);
	virtual bool Start();
	virtual void Stop(void);
};

#endif // !defined(AFX_PROXYSERVICE_H__E6318F0D_4121_43CA_A7A5_38E7772171DB__INCLUDED_)
