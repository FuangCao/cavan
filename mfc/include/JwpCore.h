#pragma once

#include "jwp.h"

class JwpCore : public jwp_desc
{
protected:
	int mLogIndex;
	jwp_bool mInitiated;
	CFile mFileLog;

	JwpCore(void);
	virtual ~JwpCore(void);

private:
	static jwp_size_t HwReadHandler(struct jwp_desc *jwp, void *buff, jwp_size_t size);
	static jwp_size_t HwWriteHandler(struct jwp_desc *jwp, const void *buff, jwp_size_t size);
	static void OnSendCompleteHandler(struct jwp_desc *jwp);
	static void OnDataReceivedHandler(struct jwp_desc *jwp, const void *buff, jwp_size_t size);
	static void OnCommandReceivedHandler(struct jwp_desc *jwp, const void *command, jwp_size_t size);
	static void OnPackageReceivedHandler(struct jwp_desc *jwp, const struct jwp_header *hdr);
	static void OnLogReceivedHandler(struct jwp_desc *jwp, jwp_device_t device, const char *log, jwp_size_t size);
	static void TxThreadHandler(void *data);
	static void RxThreadHandler(void *data);
	static void RxPackageThreadHandler(void *data);
	static void TxDataThreadHandler(void *data);

protected:
	jwp_bool StartJwp(jwp_bool useRxThread = true);
	void StopJwp(void);
	virtual int HwRead(void *buff, jwp_size_t size);
	virtual int HwWrite(const void *buff, jwp_size_t size) = 0;
	virtual void OnLogReceived(jwp_device_t device, const char *log, jwp_size_t size);

	virtual void OnSendComplete(void)
	{
		println("OnSendComplete");
	}

	virtual void OnDataReceived(const void *buff, jwp_size_t size)
	{
		println("OnDataReceived: size = %d", size);
	}

	virtual void OnCommandReceived(const void *command, jwp_size_t size)
	{
		println("OnCommandReceived: size = %d", size);
	}

	virtual void OnPackageReceived(const struct jwp_header *hdr)
	{
		println("OnPackageReceived: index = %d, type = %d, length = %d", hdr->index, hdr->type, hdr->length);
	}

public:
	void puts(const char *text, jwp_size_t size)
	{
		mFileLog.Write(text, size);
	}

	void puts(const char *text)
	{
		puts(text, strlen(text));
	}

	void println(const char *fmt, ...);
	void WriteRxData(const void *buff, jwp_size_t size);

	jwp_size_t SendData(const void *buff, jwp_size_t size)
	{
		return jwp_send_data(this, buff, size);
	}

	jwp_size_t RecvData(void *buff, jwp_size_t size)
	{
		return jwp_recv_data(this, buff, size);
	}

	jwp_bool SendCommand(const void *command, jwp_size_t size)
	{
		return jwp_send_command(this, command, size);
	}

	void SendLog(const char *log, jwp_size_t size)
	{
		jwp_send_log(this, log, size);
	}
};
