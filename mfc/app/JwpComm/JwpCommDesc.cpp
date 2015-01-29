#include "stdafx.h"
#include "JwpCommDesc.h"

int JwpCommDesc::HwWrite(const void *buff, jwp_size_t size)
{
	CByteArray bytes;
	const UCHAR *p, *p_end;

	for (p = (UCHAR *) buff, p_end = p + size; p < p_end; p++)
	{
		bytes.Add(*p);
	}

	SetOutput(COleVariant(bytes));

	return size;
}

void JwpCommDesc::WriteRxData(void)
{
	long i;
	COleSafeArray input = GetInput();
	int rdLen = input.GetOneDimSize();
	char buff[1024], *p, *last;

	for (i = 0, p = buff, last = p + sizeof(buff) - 1; i < rdLen; i++, p++)
	{
		if (p > last)
		{
			mJwp->WriteRxData(buff, sizeof(buff));
			p = buff;
		}

		input.GetElement(&i, p);
	}

	mJwp->WriteRxData(buff, p - buff);
}
