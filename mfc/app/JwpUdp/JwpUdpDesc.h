#pragma once

#include "jwp.h"

#define CAVAN_NETWORK_MAGIC		0x88888888

typedef enum
{
	JWP_UDP_CMD_CONNECT,
	JWP_UDP_CMD_COUNT
} jwp_udp_command_t;

#pragma pack(1)
struct jwp_udp_command
{
	jwp_u8 type;
};
#pragma pack()

class JwpUdpDesc : public jwp_desc
{
private:
	jwp_bool mJwpInitiated;
	jwp_bool mConnected;
	SOCKET mSocket;
	int mSockAddrLen;
	struct sockaddr_in mSockAddr;

	jwp_bool Connect(void);
	int HwRead(void *buff, jwp_size_t size);
	int HwWrite(const void *buff, jwp_size_t size);

	static jwp_size_t JwpHwRead(struct jwp_desc *jwp, void *buff, jwp_size_t size);
	static jwp_size_t JwpHwWrite(struct jwp_desc *jwp, const void *buff, jwp_size_t size);
	static void OnSendCompleteHandler(struct jwp_desc *jwp);
	static void OnDataReceivedHandler(struct jwp_desc *jwp, const void *buff, jwp_size_t size);
	static void OnCommandReceivedHandler(struct jwp_desc *jwp, const void *command, jwp_size_t size);
	static void OnPackageReceivedHandler(struct jwp_desc *jwp, const struct jwp_header *hdr);
	static void OnLogReceivedHandler(struct jwp_desc *jwp, const char *log, jwp_size_t size);

	static void TxThread(void *data);
	static void RxThread(void *data);
	static void RxPackageThread(void *data);
	static void TxDataThread(void *data);

	jwp_bool SendJwpUdpCommand(jwp_udp_command_t command);

protected:
	virtual void OnSendComplete(void) {}
	virtual void OnDataReceived(const void *buff, jwp_size_t size) {}
	virtual void OnCommandReceived(const void *command, jwp_size_t size) {}
	virtual void OnPackageReceived(const struct jwp_header *hdr) {}
	virtual void OnLogReceived(const char *log, jwp_size_t size) {}

public:
	JwpUdpDesc(void);
	void printf(const char *fmt, ...);
	jwp_bool InitJwp(void);
	jwp_bool Open(DWORD ip, WORD port);
	void Close(void);
	jwp_size_t SendData(const void *buff, jwp_size_t size);
	jwp_size_t RecvData(void *buff, jwp_size_t size);
	jwp_bool SendCommand(const void *command, jwp_size_t size);
};
