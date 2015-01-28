#pragma once

#include "JwpCore.h"

#define CAVAN_NETWORK_MAGIC		0x88888888

#pragma pack(1)
struct jwp_udp_command
{
	jwp_u8 type;
};
#pragma pack()

class JwpUdpDesc : public JwpCore
{
private:
	jwp_bool mConnected;
	SOCKET mSocket;
	int mSockAddrLen;
	struct sockaddr_in mSockAddr;

private:
	jwp_bool Connect(void);

protected:
	JwpUdpDesc(void) : mConnected(false), mSocket(INVALID_SOCKET) {}

	virtual int JwpUdpDesc::HwRead(void *buff, jwp_size_t size)
	{
		return recvfrom(mSocket, (char *) buff, size, 0, (struct sockaddr *) &mSockAddr, &mSockAddrLen);
	}

	virtual int JwpUdpDesc::HwWrite(const void *buff, jwp_size_t size)
	{
		return sendto(mSocket, (const char *) buff, size, 0, (struct sockaddr *) &mSockAddr, mSockAddrLen);
	}

public:
	jwp_bool StartJwp(DWORD ip, WORD port);
	void StopJwp(void);
};
