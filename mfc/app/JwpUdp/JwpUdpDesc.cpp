#include "stdafx.h"
#include "process.h"
#include "JwpUdp.h"
#include "JwpUdpDesc.h"

jwp_size_t JwpUdpDesc::JwpHwRead(struct jwp_desc *jwp, void *buff, jwp_size_t size)
{
	int rdlen;
	JwpUdpDesc *jwp_udp = (JwpUdpDesc *) jwp;

	rdlen = jwp_udp->HwRead(buff, size);
	if (rdlen < 0)
	{
		jwp_msleep(JWP_POLL_TIME);
		return 0;
	}

	return rdlen;
}

jwp_size_t JwpUdpDesc::JwpHwWrite(struct jwp_desc *jwp, const void *buff, jwp_size_t size)
{
	int wrlen;
	JwpUdpDesc *jwp_udp = (JwpUdpDesc *) jwp;

	wrlen = jwp_udp->HwWrite(buff, size);
	if (wrlen < 0)
	{
		jwp_msleep(JWP_POLL_TIME);
		return 0;
	}

	return wrlen;
}

void JwpUdpDesc::OnSendCompleteHandler(struct jwp_desc *jwp)
{
	JwpUdpDesc *jwp_udp = (JwpUdpDesc *) jwp;

	jwp_udp->OnSendComplete();
}

void JwpUdpDesc::OnDataReceivedHandler(struct jwp_desc *jwp, const void *buff, jwp_size_t size)
{
	JwpUdpDesc *jwp_udp = (JwpUdpDesc *) jwp;

	jwp_udp->OnDataReceived(buff, size);
}

void JwpUdpDesc::OnCommandReceivedHandler(struct jwp_desc *jwp, const void *command, jwp_size_t size)
{
	JwpUdpDesc *jwp_udp = (JwpUdpDesc *) jwp;

	jwp_udp->OnCommandReceived(command, size);
}

void JwpUdpDesc::OnPackageReceivedHandler(struct jwp_desc *jwp, const struct jwp_header *hdr)
{
	JwpUdpDesc *jwp_udp = (JwpUdpDesc *) jwp;

	jwp_udp->OnPackageReceived(hdr);
}

void JwpUdpDesc::OnLogReceivedHandler(struct jwp_desc *jwp, const char *log, jwp_size_t size)
{
	JwpUdpDesc *jwp_udp = (JwpUdpDesc *) jwp;

	jwp_udp->OnLogReceived(log, size);
}

void JwpUdpDesc::TxThread(void *data)
{
	JwpUdpDesc *jwp_udp = (JwpUdpDesc *) data;

#if JWP_TX_LOOP_ENABLE
	jwp_tx_loop(jwp_udp);
#endif
}

void JwpUdpDesc::RxThread(void *data)
{
	JwpUdpDesc *jwp_udp = (JwpUdpDesc *) data;

#if JWP_RX_LOOP_ENABLE
	jwp_rx_loop(jwp_udp);
#endif
}

void JwpUdpDesc::RxPackageThread(void *data)
{
	JwpUdpDesc *jwp_udp = (JwpUdpDesc *) data;

#if JWP_RX_PKG_LOOP_ENABLE
	jwp_rx_package_loop(jwp_udp);
#endif
}

void JwpUdpDesc::TxDataThread(void *data)
{
	JwpUdpDesc *jwp_udp = (JwpUdpDesc *) data;

#if JWP_TX_DATA_LOOP_ENABLE
	jwp_tx_data_loop(jwp_udp);
#endif
}

JwpUdpDesc::JwpUdpDesc(void)
{
	hw_read = JwpHwRead;
	hw_write = JwpHwWrite;
	send_complete = OnSendCompleteHandler;
	data_received = OnDataReceivedHandler;
	command_received = OnCommandReceivedHandler;
	package_received = OnPackageReceivedHandler;
#if JWP_PRINTF_ENABLE
	log_received = OnLogReceivedHandler;
#endif

	mSocket = INVALID_SOCKET;
	mJwpInitiated = false;
	mConnected = false;
}

void JwpUdpDesc::printf(const char *fmt, ...)
{
	int size;
	va_list ap;
	char buff[2048];

	va_start(ap, fmt);
	size = _vsnprintf(buff, sizeof(buff), fmt, ap);
	va_end(ap);

	OnLogReceived(buff, size);
}

jwp_bool JwpUdpDesc::SendJwpUdpCommand(jwp_udp_command_t type)
{	
	struct jwp_udp_command command;

	command.type = type;

	return SendCommand(&command, sizeof(command));
}

jwp_bool JwpUdpDesc::InitJwp(void)
{
	if (mJwpInitiated)
	{
		return true;
	}

	mJwpInitiated = jwp_init(this, this);
	if (!mJwpInitiated)
	{
		return false;
	}
	
#if JWP_TX_LOOP_ENABLE
	_beginthread(TxThread, 0, this);
#endif

#if JWP_RX_LOOP_ENABLE
	_beginthread(RxThread, 0, this);
#endif

#if JWP_RX_PKG_LOOP_ENABLE
	_beginthread(RxPackageThread, 0, this);
#endif

#if JWP_TX_DATA_LOOP_ENABLE
	_beginthread(TxDataThread, 0, this);
#endif

	return true;
}

jwp_bool JwpUdpDesc::Connect(void)
{
	int length;
	jwp_u32 magic = CAVAN_NETWORK_MAGIC;

	length = HwWrite(&magic, sizeof(magic));
	if (length != sizeof(magic))
	{
		printf("HwWrite failed %d\n", length);
		return false;
	}

	length = HwRead(&magic, sizeof(magic));
	if (length != sizeof(magic))
	{
		printf("HwRead failed %d\n", length);
		return false;
	}

	if (magic != CAVAN_NETWORK_MAGIC)
	{
		printf("Invalid magic 0x%08x\n", magic);
		return false;
	}

	return true;
}

int JwpUdpDesc::HwRead(void *buff, jwp_size_t size)
{
	return recvfrom(mSocket, (char *) buff, size, 0, (struct sockaddr *) &mSockAddr, &mSockAddrLen);
}

int JwpUdpDesc::HwWrite(const void *buff, jwp_size_t size)
{
	return sendto(mSocket, (const char *) buff, size, 0, (struct sockaddr *) &mSockAddr, mSockAddrLen);
}

jwp_bool JwpUdpDesc::Open(DWORD ip, WORD port)
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

	if (!InitJwp())
	{
		goto out_closesocket;
	}

	return true;

out_closesocket:
	closesocket(mSocket);
	mSocket = INVALID_SOCKET;

	return false;
}

void JwpUdpDesc::Close(void)
{
	if (mSocket != INVALID_SOCKET)
	{
		closesocket(mSocket);
		mSocket = INVALID_SOCKET;
	}

	mConnected = false;
}

jwp_size_t JwpUdpDesc::SendData(const void *buff, jwp_size_t size)
{
	return jwp_send_data(this, buff, size);
}

jwp_size_t JwpUdpDesc::RecvData(void *buff, jwp_size_t size)
{
	return jwp_recv_data(this, buff, size);
}

jwp_bool JwpUdpDesc::SendCommand(const void *command, jwp_size_t size)
{
	return jwp_send_command(this, command, size);
}
