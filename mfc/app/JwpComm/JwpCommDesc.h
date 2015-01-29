#pragma once

#include "mscomm.h"
#include "JwpCore.h"

class JwpCommDesc : public CMSComm
{
private:
	JwpCore *mJwp;

public:
	void setJwp(JwpCore *jwp)
	{
		mJwp = jwp;
	}

	int HwWrite(const void *buff, jwp_size_t size);	
	void WriteRxData(void);
};
