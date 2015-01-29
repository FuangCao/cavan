#include "stdafx.h"
#include "process.h"
#include "JwpUdp.h"
#include "JwpUdpDesc.h"

jwp_bool JwpUdpDesc::Connect(void)
{
	int length;
	jwp_u32 magic = CAVAN_NETWORK_MAGIC;

	length = HwWrite(&magic, sizeof(magic));
	if (length != sizeof(magic))
	{
		println("HwWrite failed %d", length);
		return false;
	}

#if 0
	length = HwRead(&magic, sizeof(magic));
	if (length != sizeof(magic))
	{
		println("HwRead failed %d", length);
		return false;
	}

	if (magic != CAVAN_NETWORK_MAGIC)
	{
		println("Invalid magic 0x%08x", magic);
		return false;
	}
#endif

	return true;
}

jwp_bool JwpUdpDesc::StartJwp(DWORD ip, WORD port)
{
	if (mSocket != INVALID_SOCKET)
	{
		return true;
	}

	mSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (mSocket == INVALID_SOCKET)
	{
		return false;
	}

	mSockAddr.sin_family = AF_INET;
	mSockAddr.sin_addr.s_addr = htonl(ip);
	mSockAddr.sin_port = htons(port);

	mSockAddrLen = sizeof(mSockAddr);

	if (!Connect())
	{
		goto out_closesocket;
	}

	if (!JwpCore::StartJwp())
	{
		goto out_closesocket;
	}

	return true;

out_closesocket:
	closesocket(mSocket);
	mSocket = INVALID_SOCKET;

	return false;
}

void JwpUdpDesc::StopJwp(void)
{
	if (mSocket != INVALID_SOCKET)
	{
		closesocket(mSocket);
		mSocket = INVALID_SOCKET;
	}

	mConnected = false;
}
