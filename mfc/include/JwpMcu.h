#pragma once

#include "JwpCore.h"
#include "jwp_mcu.h"

class JwpMcu : public JwpCore, jwp_mcu_desc
{
private:
	jwp_bool mInitiated;
	static void CsrStateChangedHandler(struct jwp_mcu_desc *mcu, const struct jwp_csr_event_state *event);

protected:
	virtual void OnCsrStateChanged(const struct jwp_csr_event_state *event) {}
	virtual jwp_bool JwpInit(void);
	virtual jwp_bool JwpStart(jwp_bool useRxThread = true);

public:
	static const char *JwpMcu::CsrStateToString(jwp_u8 state);
	bool GetBonded(void)
	{
		return csr_bonded > 0;
	}

	app_state GetCsrState(void)
	{
		return csr_state;
	}

	const char *GetCsrStateString(void)
	{
		return CsrStateToString(csr_state);
	}

public:
	JwpMcu(void);

	jwp_bool CsrSendCommand(const void *command, jwp_size_t size)
	{
		return jwp_csr_send_command(this, command, size);
	}

	jwp_bool CsrSendEmptyCommand(jwp_u8 type)
	{
		return jwp_csr_send_empty_command(this, type);
	}

	jwp_bool CsrRemovePair(void)
	{
		return jwp_csr_remove_pair(this);
	}

	jwp_bool CsrSendGetStateCommand(void)
	{
		return jwp_csr_get_state(this);
	}

	jwp_bool CsrGetFirmwareInfo(void)
	{
		return jwp_csr_get_firmware_info(this);
	}

	jwp_bool CsrStartAdvert(void)
	{
		return jwp_csr_start_advert(this);
	}

	jwp_bool CsrDirectedAdvert(void)
	{
		return jwp_csr_directed_advert(this);
	}

	jwp_bool CsrDisconnect(void)
	{
		return jwp_csr_disconnect(this);
	}

	jwp_bool CsrSetIdle(void)
	{
		return jwp_csr_set_idle(this);
	}
};
