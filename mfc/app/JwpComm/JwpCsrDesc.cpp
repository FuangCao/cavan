#include "stdafx.h"
#include "JwpCsrDesc.h"

jwp_bool JwpCsrDesc::JwpStart(jwp_bool useRxThread )
{
	if (!JwpCore::JwpStart(useRxThread))
	{
		return false;
	}

	return SendCsrStateRequest();
}

const char *JwpCsrDesc::CsrStateToString(jwp_u8 state)
{
	switch (state)
	{
	case app_state_init:
		return "������ʼ��";

	case app_state_fast_advertising:
		return "�������ٹ㲥";

	case app_state_slow_advertising:
		return "�������ٹ㲥";

	case app_state_directed_advertising:
		return "��������㲥";

	case app_state_connected:
		return "����������";

	case app_state_disconnecting:
		return "�����ѶϿ�����";

	case app_state_idle:
		return "��������";

	default:
		return "δ֪";
	}
}

void JwpCsrDesc::OnCommandReceived(const void *command, jwp_size_t size)
{
	if (size == sizeof(struct jwp_csr_command_package))
	{
		struct jwp_csr_command_package *pkg = (struct jwp_csr_command_package *) command;
		switch (pkg->type)
		{
		case JWP_CSR_EVENT_STATE:
			mCsrState = (app_state) pkg->code;
			OnCsrStateChanged(mCsrState);
			break;

		default:
			println("Invalid csr command %d", pkg->type);
		}
	}
	else
	{
		println("Invalid command size %d", size);
	}
}

jwp_bool JwpCsrDesc::SendCsrCommand(jwp_u8 type, jwp_u8 code)
{
	struct jwp_csr_command_package pkg;

	pkg.type = type;
	pkg.code = code;

	return SendCommand(&pkg, sizeof(pkg));
}
