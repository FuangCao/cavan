// ProxyService.h: interface for the CProxyService class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROXYSERVICE_H__E6318F0D_4121_43CA_A7A5_38E7772171DB__INCLUDED_)
#define AFX_PROXYSERVICE_H__E6318F0D_4121_43CA_A7A5_38E7772171DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CavanService.h"

#define ADB_SERVICE_PORT	5037

class CProxyThread;

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
	SOCKET mSocket;
	struct sockaddr_in mSockAddr;
	int mSockAddrLen;

public:
	CCavanTransport(SOCKET sockfd, struct sockaddr_in *addr, bool bNeedClose = true);
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
	CTransportTcpClient(SOCKET sockfd = INVALID_SOCKET, struct sockaddr_in *addr = NULL) : CCavanTransport(sockfd, addr) {}
	virtual bool Open(WORD port, DWORD ip = INADDR_LOOPBACK);
	virtual int ReceiveData(void *buff, int size);
	virtual int SendData(const void *buff, int size);
};

class CTransportAdbClient : public CTransportTcpClient
{
private:
	char mStatus[1024];

public:
	virtual bool Open(WORD port = 8888, DWORD ip = INADDR_ANY);

	bool Connect(WORD port);
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
	CTransportUdpClient(SOCKET sockfd = INVALID_SOCKET, struct sockaddr_in *addr = NULL, bool bNeedClose = true) : CCavanTransport(sockfd, addr, bNeedClose) {}
	virtual bool Open(WORD port, DWORD ip = INADDR_LOOPBACK);
	virtual int ReceiveData(void *buff, int size);
	virtual int SendData(const void *buff, int size);
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
	CCavanTransport *mProxyTransport;
	WORD mProxyPort;
	DWORD mProxyIP;
	CProxyProcotolType mProxyProtocol;

protected:
	virtual bool Run(void);

public:
	CProxyThread() : CCavanThread(), mProxyTransport(NULL) {}

	virtual void Prepare(CCavanTransport *trspService, WORD wProxyPort = 8888, CProxyProcotolType nProxyProtocol = PROXY_PROTOCOL_TYPE_ADB, DWORD dwProxyIP = INADDR_LOOPBACK);
	virtual bool Start(int index);
	virtual void Stop(void);
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

public:
	CProxyService() : CCavanService(), mLocalPort(8888), mProxyPort(8888), mProxyIP(INADDR_LOOPBACK), mLocalProtocol(PROXY_PROTOCOL_TYPE_TCP), mProxyProtocol(PROXY_PROTOCOL_TYPE_ADB), mServiceTransport(NULL) {}
	virtual ~CProxyService();

	void Prepare(WORD wProxyPort, WORD wLocalPort = 8888, DWORD dwProxyIP = INADDR_LOOPBACK, CProxyProcotolType nLocalProtocol = PROXY_PROTOCOL_TYPE_TCP, CProxyProcotolType nProxyProtocol = PROXY_PROTOCOL_TYPE_ADB, int nDaemonCount = 20);
	bool CreateThreads(void);
	virtual bool Start(void);
	virtual void Stop(void);
};

#endif // !defined(AFX_PROXYSERVICE_H__E6318F0D_4121_43CA_A7A5_38E7772171DB__INCLUDED_)
