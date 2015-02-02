#include "stdafx.h"
#include "JwpMcu.h"

void JwpMcu::CsrStateChangedHandler(struct jwp_mcu_desc *mcu_desc, const struct jwp_csr_event_state *event)
{
	JwpMcu *mcu = (JwpMcu *) mcu_desc;

	mcu->OnCsrStateChanged(event);
}

const char *JwpMcu::CsrStateToString(jwp_u8 state)
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

JwpMcu::JwpMcu(void) : JwpCore()
{
	mInitiated = false;

	csr_state_changed = CsrStateChangedHandler;
}

jwp_bool JwpMcu::JwpInit(void)
{
	if (mInitiated)
	{
		return true;
	}

	if (!JwpCore::JwpInit())
	{
		return false;
	}

	mInitiated = jwp_mcu_init(this, this);

	return mInitiated;
}
