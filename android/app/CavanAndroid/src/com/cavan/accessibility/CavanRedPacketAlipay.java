package com.cavan.accessibility;

public class CavanRedPacketAlipay extends CavanRedPacket {

	private String mCode;

	public CavanRedPacketAlipay(String code) {
		mCode = code;
	}

	public String getCode() {
		return mCode;
	}

	public void setCode(String code) {
		mCode = code;
	}

	@Override
	public boolean equals(Object o) {
		if (super.equals(o)) {
			return true;
		}

		if (o instanceof CavanRedPacketAlipay) {
			return mCode.equals(((CavanRedPacketAlipay) o).getCode());
		}

		if (o instanceof String) {
			return mCode.equals(o);
		}

		return false;
	}

}
