#include "stdafx.h"
#include "process.h"
#include "JwpCore.h"

jwp_size_t JwpCore::HwReadHandler(struct jwp_desc *jwp, void *buff, jwp_size_t size)
{
	int rdlen;
	JwpCore *jwp_core = (JwpCore *) jwp;

	rdlen = jwp_core->HwRead(buff, size);
	if (rdlen < 0)
	{
		jwp_msleep(JWP_POLL_TIME);
		return 0;
	}

	return rdlen;
}

jwp_size_t JwpCore::HwWriteHandler(struct jwp_desc *jwp, const void *buff, jwp_size_t size)
{
	int wrlen;
	JwpCore *jwp_core = (JwpCore *) jwp;

	wrlen = jwp_core->HwWrite(buff, size);
	if (wrlen < 0)
	{
		jwp_msleep(JWP_POLL_TIME);
		return 0;
	}

	return wrlen;
}

// ======================================================

void JwpCore::OnSendCompleteHandler(struct jwp_desc *jwp)
{
	JwpCore *jwp_core = (JwpCore *) jwp;

	jwp_core->OnSendComplete();
}

void JwpCore::OnDataReceivedHandler(struct jwp_desc *jwp, const void *buff, jwp_size_t size)
{
	JwpCore *jwp_core = (JwpCore *) jwp;

	jwp_core->OnDataReceived(buff, size);
}

void JwpCore::OnCommandReceivedHandler(struct jwp_desc *jwp, const void *command, jwp_size_t size)
{
	JwpCore *jwp_core = (JwpCore *) jwp;

	jwp_core->OnCommandReceived(command, size);
}

void JwpCore::OnPackageReceivedHandler(struct jwp_desc *jwp, const struct jwp_header *hdr)
{
	JwpCore *jwp_core = (JwpCore *) jwp;

	jwp_core->OnPackageReceived(hdr);
}

void JwpCore::OnLogReceivedHandler(struct jwp_desc *jwp, const char *log, jwp_size_t size)
{
	JwpCore *jwp_core = (JwpCore *) jwp;

	jwp_core->OnLogReceived(log, size);
}

// ======================================================

void JwpCore::TxThreadHandler(void *data)
{
	JwpCore *jwp_core = (JwpCore *) data;

#if JWP_TX_LOOP_ENABLE
	jwp_tx_loop(jwp_core);
#endif
}

void JwpCore::RxThreadHandler(void *data)
{
	JwpCore *jwp_core = (JwpCore *) data;

#if JWP_RX_LOOP_ENABLE
	jwp_rx_loop(jwp_core);
#endif
}

void JwpCore::RxPackageThreadHandler(void *data)
{
	JwpCore *jwp_core = (JwpCore *) data;

#if JWP_RX_PKG_LOOP_ENABLE
	jwp_rx_package_loop(jwp_core);
#endif
}

void JwpCore::TxDataThreadHandler(void *data)
{
	JwpCore *jwp_core = (JwpCore *) data;

#if JWP_TX_DATA_LOOP_ENABLE
	jwp_tx_data_loop(jwp_core);
#endif
}

// ======================================================

JwpCore::JwpCore(void)
{
	mInitiated = false;

	hw_read = HwReadHandler;
	hw_write = HwWriteHandler;
	send_complete = OnSendCompleteHandler;
	data_received = OnDataReceivedHandler;
	command_received = OnCommandReceivedHandler;
	package_received = OnPackageReceivedHandler;
#if JWP_PRINTF_ENABLE
	log_received = OnLogReceivedHandler;
#endif

	mFileLog.Open("jwp-core-log.txt", CFile::modeWrite | CFile::modeCreate | CFile::shareDenyWrite, NULL);
}

JwpCore::~JwpCore()
{
}

// ======================================================

void JwpCore::println(const char *fmt, ...)
{
	int size;
	va_list ap;
	char buff[2048];

	va_start(ap, fmt);
	size = _vsnprintf(buff, sizeof(buff), fmt, ap);
	va_end(ap);

	buff[size++] = '\n';

	mFileLog.Write(buff, size);
}

jwp_bool JwpCore::StartJwp(jwp_bool useRxThread)
{
	if (mInitiated)
	{
		return true;
	}

	mInitiated = jwp_init(this, this);
	if (!mInitiated)
	{
		return false;
	}

#if JWP_TX_LOOP_ENABLE
	_beginthread(TxThreadHandler, 0, this);
#endif

#if JWP_RX_LOOP_ENABLE
	if (useRxThread)
	{
		_beginthread(RxThreadHandler, 0, this);
	}
#endif

#if JWP_RX_PKG_LOOP_ENABLE
	_beginthread(RxPackageThreadHandler, 0, this);
#endif

#if JWP_TX_DATA_LOOP_ENABLE
	_beginthread(TxDataThreadHandler, 0, this);
#endif

	return true;
}

void JwpCore::StopJwp(void)
{
}

int JwpCore::HwRead(void *buff, jwp_size_t size)
{
	while (1)
	{
		println("HwRead is not implement!");

		jwp_msleep(2000);
	}
}

void JwpCore::WriteRxData(const void *buff, jwp_size_t size)
{
	const char *p, *p_end;

	for (p = (char *) buff, p_end = p + size; p < p_end;)
	{
		jwp_size_t wrlen;

		wrlen = jwp_write_rx_data(this, (jwp_u8 *) buff, size);
		if (wrlen > 0)
		{
			p += wrlen;
		}
		else
		{
			jwp_msleep(200);
		}
	}
}

jwp_size_t JwpCore::SendData(const void *buff, jwp_size_t size)
{
	return jwp_send_data(this, buff, size);
}

jwp_size_t JwpCore::RecvData(void *buff, jwp_size_t size)
{
	return jwp_recv_data(this, buff, size);
}

jwp_bool JwpCore::SendCommand(const void *command, jwp_size_t size)
{
	return jwp_send_command(this, command, size);
}

void JwpCore::SendLog(const char *log, jwp_size_t size)
{
	jwp_send_log(this, log, size);
}
