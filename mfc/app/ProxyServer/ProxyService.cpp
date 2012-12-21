// ProxyService.cpp: implementation of the CProxyService class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ProxyServer.h"
#include "ProxyService.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCavanTransport::CCavanTransport(SOCKET sockfd, struct sockaddr_in *addr, bool bNeedClose)
{
	mSocket = sockfd;

	if (addr)
	{
		mSockAddr = *addr;
	}

	mSockAddrLen = sizeof(mSockAddr);
	mNeedClose = bNeedClose;
}

void CCavanTransport::Close(void)
{
	if (mNeedClose && mSocket != INVALID_SOCKET)
	{
		closesocket(mSocket);
		mSocket = INVALID_SOCKET;
	}
}

// ================================================================================

bool CTransportTcpClient::Open(WORD port, DWORD ip)
{
	mSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (mSocket == INVALID_SOCKET)
	{
		return false;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(ip);
	addr.sin_port = htons(port);

	if (connect(mSocket, (struct sockaddr *)&addr, sizeof(addr)) != 0)
	{
		CCavanTransport::Close();
		return false;
	}

	return true;
}

int CTransportTcpClient::ReceiveData(void *buff, int size)
{
	return recv(mSocket, (char *)buff, size, 0);
}

int CTransportTcpClient::SendData(const void *buff, int size)
{
	return send(mSocket, (char *)buff, size, 0);
}

// ================================================================================

bool CTransportAdbClient::CheckStatus(void)
{
	bool bRetValue;
	int nRdLen;

	nRdLen = ReceiveData(mStatus, 8);
	if (nRdLen < 0)
	{
		return false;
	}

	mStatus[nRdLen] = 0;
	bRetValue = TextLhCmp("OKAY", mStatus) ? false : true;

	if (nRdLen == 8)
	{
		DWORD dwLength = TextToValue(mStatus + 4, 16);
		if (dwLength >= sizeof(mStatus))
		{
			dwLength = sizeof(mStatus) - 1;
		}

		nRdLen = ReceiveData(mStatus, dwLength);
		if (nRdLen < 0)
		{
			return false;
		}

		mStatus[nRdLen] = 0;
	}

	return bRetValue;
}

bool CTransportAdbClient::SendText(const char *text)
{
	int ret;
	char buff[8];
	int nLength = strlen(text);

	sprintf(buff, "%04x", nLength);
	ret = SendData(buff, 4);
	if (ret < 0)
	{
		return false;
	}

	ret = SendData(text, nLength);
	if (ret < 0)
	{
		return false;
	}

	if (CheckStatus() == false)
	{
		return false;
	}

	return true;
}

bool CTransportAdbClient::Connect(WORD port)
{
	if (SendText("host:transport-any") == false)
	{
		return false;
	}

	char buff[32];
	sprintf(buff, "tcp:%04d", port);

	return SendText(buff);
}

bool CTransportAdbClient::Open(WORD port, DWORD ip)
{
	if (CTransportTcpClient::Open(ADB_SERVICE_PORT, ip) == false)
	{
		return false;
	}

	if (Connect(port) == false)
	{
		CTransportTcpClient::Close();
		return false;
	}

	return true;
}

// ================================================================================

bool CTransportTcpService::Open(WORD port, DWORD ip)
{
	mSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (mSocket == INVALID_SOCKET)
	{
		return false;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(ip);
	addr.sin_port = htons(port);

	if (bind(mSocket, (struct sockaddr *)&addr, sizeof(addr)) != 0)
	{
		Close();
		return false;
	}

	if (listen(mSocket, 0) != 0)
	{
		Close();
		return false;
	}

	return true;
}

CCavanTransport *CTransportTcpService::Accept(void *buff, int *size)
{
	mSockAddrLen = sizeof(mSockAddr);

	SOCKET sockfd = accept(mSocket, (struct sockaddr *)&mSockAddr, &mSockAddrLen);
	if (sockfd == INVALID_SOCKET)
	{
		return NULL;
	}

	CCavanTransport *transport = new CTransportTcpClient(sockfd, &mSockAddr);
	if (transport == NULL)
	{
		closesocket(sockfd);
		return NULL;
	}

	if (size)
	{
		*size = 0;
	}

	return transport;
}

// ================================================================================

bool CTransportUdpClient::Open(WORD port, DWORD ip)
{
	mSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (mSocket == INVALID_SOCKET)
	{
		return false;
	}

	mSockAddr.sin_family = AF_INET;
	mSockAddr.sin_addr.s_addr = htonl(ip);
	mSockAddr.sin_port = htons(port);

	mSockAddrLen = sizeof(mSockAddr);

	return true;
}

int CTransportUdpClient::ReceiveData(void *buff, int size)
{
	return recvfrom(mSocket, (char *)buff, size, 0, (struct sockaddr *)&mSockAddr, &mSockAddrLen);
}

int CTransportUdpClient::SendData(const void *buff, int size)
{
	return sendto(mSocket, (char *)buff, size, 0, (struct sockaddr *)&mSockAddr, mSockAddrLen);
}

// ================================================================================

bool CTransportUdpService::Open(WORD port, DWORD ip)
{
	mSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (mSocket == INVALID_SOCKET)
	{
		return false;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	if (bind(mSocket, (struct sockaddr *)&addr, sizeof(addr)) != 0)
	{
		Close();
		return false;
	}

	return true;
}

CCavanTransport *CTransportUdpService::Accept(void *buff, int *size)
{
	int nRecvLen = ReceiveData(buff, *size);
	if (nRecvLen < 0)
	{
		return NULL;
	}

	CCavanTransport *transport = new CTransportUdpClient(mSocket, &mSockAddr, false);
	if (transport == NULL)
	{
		return NULL;
	}

	return transport;
}

// ================================================================================

void CProxyThread::Prepare(CCavanTransport *trspService, WORD wProxyPort, CProxyProcotolType nProxyProtocol, DWORD dwProxyIP)
{
	mServiceTransport = trspService;
	mProxyPort = wProxyPort;
	mProxyProtocol = nProxyProtocol;
	mProxyIP = dwProxyIP;
}

bool CProxyThread::Run(void)
{
	char buff[1024];
	int nLength = sizeof(buff);

	CCavanTransport *trspClient = mServiceTransport->Accept(buff, &nLength);
	if (trspClient == NULL)
	{
		return false;
	}

	if (mProxyTransport->Open(mProxyPort, mProxyIP) == false)
	{
		struct in_addr addr;
		addr.s_addr = htonl(mProxyIP);
		CavanMessageBoxError("Open: IP = %s, Port = %d", inet_ntoa(addr), mProxyPort);
		delete trspClient;
		return true;
	}

	if (nLength && mProxyTransport->SendData(buff, nLength) < 0)
	{
		mProxyTransport->Close();
		delete trspClient;
		return true;
	}

	fd_set setRead;

	FD_ZERO(&setRead);
	FD_SET(trspClient->mSocket, &setRead);
	FD_SET(mProxyTransport->mSocket, &setRead);

	int nFdCount = max(trspClient->mSocket, mProxyTransport->mSocket) + 1;

	while (1)
	{
		int ret = select(nFdCount, &setRead, NULL, NULL, NULL);
		if (ret <= 0)
		{
			break;
		}

		if (FD_ISSET(trspClient->mSocket, &setRead))
		{
			nLength = trspClient->ReceiveData(buff, sizeof(buff));
			if (nLength <= 0 || mProxyTransport->SendData(buff, nLength) < 0)
			{
				break;
			}
		}
		else
		{
			FD_SET(trspClient->mSocket, &setRead);
		}

		if (FD_ISSET(mProxyTransport->mSocket, &setRead))
		{
			nLength = mProxyTransport->ReceiveData(buff, sizeof(buff));
			if (nLength <= 0 || trspClient->SendData(buff, nLength) < 0)
			{
				break;
			}
		}
		else
		{
			FD_SET(mProxyTransport->mSocket, &setRead);
		}
	}

	mProxyTransport->Close();
	delete trspClient;

	return true;
}

bool CProxyThread::Start(int index)
{
	CSingleLock lock(&mLock);

	if (mServiceTransport == NULL || mProxyTransport)
	{
		return false;
	}

	switch (mProxyProtocol)
	{
	case PROXY_PROTOCOL_TYPE_TCP:
		mProxyTransport = new CTransportTcpClient();
		break;

	case PROXY_PROTOCOL_TYPE_UDP:
		mProxyTransport = new CTransportUdpClient();
		break;

	case PROXY_PROTOCOL_TYPE_ADB:
		mProxyTransport = new CTransportAdbClient();
		break;

	default:
		mProxyTransport = NULL;
		return false;
	}

	return CCavanThread::Start(index);
}

void CProxyThread::Stop(void)
{
	CSingleLock lock(&mLock);

	if (mProxyTransport)
	{
		mProxyTransport->Close();
	}

	CCavanThread::Stop();

	if (mProxyTransport)
	{
		delete mProxyTransport;
		mProxyTransport = NULL;
	}
}

// ================================================================================

CProxyService::~CProxyService()
{
	Stop();

	DestoryThreads();
}

bool CProxyService::CreateThreads(void)
{
	if (mThreadHead)
	{
		return true;
	}

	for (int i = 0; i < mDaemonCount; i++)
	{
		CProxyThread *thread = new CProxyThread();
		if (thread == NULL)
		{
			continue;
		}

		thread->next = mThreadHead;
		mThreadHead = thread;
	}

	return true;
}

void CProxyService::Prepare(WORD wProxyPort, WORD wLocalPort, DWORD dwProxyIP, CProxyProcotolType nLocalProtocol, CProxyProcotolType nProxyProtocol, int nDaemonCount)
{
	Stop();

	CSingleLock lock(&mLock);

	mProxyPort = wProxyPort;
	mLocalPort = wLocalPort;
	mProxyIP = dwProxyIP;
	mLocalProtocol = nLocalProtocol;
	mProxyProtocol = nProxyProtocol;

	if (mDaemonCount != nDaemonCount)
	{
		DestoryThreads();
	}

	mDaemonCount = nDaemonCount;
}

bool CProxyService::Start(void)
{
	CSingleLock lock(&mLock);

	switch (mLocalProtocol)
	{
	case PROXY_PROTOCOL_TYPE_TCP:
		mServiceTransport = new CTransportTcpService();
		break;

	case PROXY_PROTOCOL_TYPE_UDP:
		mServiceTransport = new CTransportUdpService();
		break;

	default:
		return false;
	}

	if (mServiceTransport == NULL)
	{
		return false;
	}

	if (CreateThreads() == false)
	{
		return false;
	}

	for (CCavanThread *thread = mThreadHead; thread; thread = thread->next)
	{
		thread->Prepare(mServiceTransport, mProxyPort, mProxyProtocol, mProxyIP);
	}

	if (mServiceTransport->Open(mLocalPort, INADDR_ANY) == false)
	{
		return false;
	}

	if (CCavanService::Start() == false)
	{
		mServiceTransport->Close();
		return false;
	}

	return true;
}

void CProxyService::Stop(void)
{
	CSingleLock lock(&mLock);

	if (mServiceTransport)
	{
		mServiceTransport->Close();
		CCavanService::Stop();

		delete mServiceTransport;
		mServiceTransport = NULL;
	}

	DestoryThreads();
}
