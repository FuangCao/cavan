package com.cavan.android;

import java.util.UUID;

import android.bluetooth.BluetoothDevice;
import android.content.Context;

public class CavanBleUart extends CavanBleGatt {

	public static final UUID UUID_SERVICE = UUID.fromString("0783b03e-8535-b5a0-7140-a304d2495cb7");
	public static final UUID UUID_RX = UUID.fromString("0783b03e-8535-b5a0-7140-a304d2495cba");
	public static final UUID UUID_TX = UUID.fromString("0783b03e-8535-b5a0-7140-a304d2495cb8");
	public static final UUID UUID_OTA = UUID.fromString("0783b03e-8535-b5a0-7140-a304d2495cbb");

	private CavanBleChar mCharacteristicTx;
	private CavanBleChar mCharacteristicRx;
	private CavanBleChar mCharacteristicOta;
	private CavanBleDataListener mTxDataLinstener;

	public CavanBleUart(Context context, BluetoothDevice device, UUID uuid) throws Exception {
		super(context, device, uuid);
	}

	public CavanBleUart(Context context, BluetoothDevice device) throws Exception {
		this(context, device, UUID_SERVICE);
	}

	public boolean sendData(byte[] data) {
		return mCharacteristicRx != null && mCharacteristicRx.writeData(data, true);
	}

	public boolean sendText(String text) {
		return sendData(text.getBytes());
	}

	public boolean writeOta(byte[] data) {
		return mCharacteristicOta != null && mCharacteristicOta.writeData(data, true);
	}

	public void setDataListener(CavanBleDataListener listener) {
		mTxDataLinstener = listener;
		if (mCharacteristicTx != null) {
			mCharacteristicTx.setDataListener(listener);
		}
	}

	@Override
	protected boolean doInit() {
		mCharacteristicRx = openChar(UUID_RX);
		if (mCharacteristicRx == null) {
			return false;
		}

		mCharacteristicTx = openChar(UUID_TX);
		if (mCharacteristicTx == null) {
			return false;
		}

		mCharacteristicTx.setDataListener(mTxDataLinstener);

		mCharacteristicOta = openChar(UUID_OTA);

		return true;
	}
}
