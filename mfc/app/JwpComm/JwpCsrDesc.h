#pragma once

#include "JwpCore.h"

typedef enum
{
    /* Application initial state */
    app_state_init = 0,

    /* Application is performing fast undirected advertisements */
    app_state_fast_advertising,

    /* Application is performing slow undirected advertisements */
    app_state_slow_advertising,

    /* Application is performing directed advertisements */
    app_state_directed_advertising,

    /* Connection has been established with the host */
    app_state_connected,

    /* Disconnection initiated by the application */
    app_state_disconnecting,

    /* Application is neither advertising nor connected to a host */
    app_state_idle

} app_state;

typedef enum
{
	JWP_CSR_CMD_SET_STATE,
	JWP_CSR_CMD_GET_STATE,
	JWP_CSR_CMD_RM_PAIR,
	JWP_CSR_EVENT_STATE,
} jwp_csr_command_t;

struct jwp_csr_command_package
{
	jwp_u8 type;
	jwp_u8 code;
};

class JwpCsrDesc : public JwpCore
{
private:
	app_state mCsrState;

public:
	JwpCsrDesc(void) : mCsrState(app_state_init) {}

	const char *CsrStateToString(jwp_u8 state);
	jwp_bool SendCsrCommand(jwp_u8 type, jwp_u8 code);
	jwp_bool StartJwp(jwp_bool useRxThread = false);

	jwp_size_t SendData(const void *buff, jwp_size_t size)
	{
		if (mCsrState != app_state_connected)
		{
			return 0;
		}

		return JwpCore::SendData(buff, size);
	}

	app_state GetCsrState(void)
	{
		return mCsrState;
	}

	const char *GetCsrStateString(void)
	{
		return CsrStateToString(mCsrState);
	}

	jwp_bool SendCsrState(app_state state)
	{
		return SendCsrCommand(JWP_CSR_CMD_SET_STATE, state);
	}

	jwp_bool JwpCsrDesc::CsrStartAdvert(void) 
	{
		return SendCsrState(app_state_fast_advertising);
	}

	jwp_bool JwpCsrDesc::CsrDisconnect(void) 
	{
		return SendCsrState(app_state_disconnecting);
	}

	jwp_bool JwpCsrDesc::CsrSetIdle(void) 
	{
		return SendCsrState(app_state_idle);
	}

	jwp_bool SendCsrStateRequest(void)
	{
		return SendCsrCommand(JWP_CSR_CMD_GET_STATE, 0);
	}

	jwp_bool CsrPairingRemoval(void)
	{
		return SendCsrCommand(JWP_CSR_CMD_RM_PAIR, 0);
	}

protected:
	virtual void OnCommandReceived(const void *command, jwp_size_t size);
	virtual void OnCsrStateChanged(app_state state) {}
};
