package com.cavan.cavanutils;

import java.util.UUID;

import android.bluetooth.BluetoothDevice;

public class CavanBleUart extends CavanBleGatt {

	public static final UUID SPS_SERVICE_UUID = UUID.fromString("0783b03e-8535-b5a0-7140-a304d2495cb7");
    public static final UUID SPS_RX_UUID = UUID.fromString("0783b03e-8535-b5a0-7140-a304d2495cba");
    public static final UUID SPS_TX_UUID = UUID.fromString("0783b03e-8535-b5a0-7140-a304d2495cb8");
    public static final UUID SPS_OTA_UUID = UUID.fromString("0783b03e-8535-b5a0-7140-a304d2495cbb");

	private CavanBleChar mCharacteristicTx;
	private CavanBleChar mCharacteristicRx;
	private CavanBleChar mCharacteristicOta;

	public CavanBleUart(BluetoothDevice device, UUID uuid) {
		super(device, uuid);
	}

	public CavanBleUart(BluetoothDevice device) {
		this(device, SPS_SERVICE_UUID);
	}

	public boolean sendData(byte[] data) {
		return mCharacteristicRx != null && mCharacteristicRx.writeData(data, true);
	}

	public boolean sendText(String text) {
		return sendData(text.getBytes());
	}

	public boolean writeOta(byte[] data) {
		return mCharacteristicOta != null && mCharacteristicOta.writeData(data, 128);
	}

	@Override
	protected boolean doInit() {
		mCharacteristicRx = openWriteChar(SPS_RX_UUID);
		mCharacteristicTx = openReadChar(SPS_TX_UUID);
		if (mCharacteristicRx == null || mCharacteristicTx == null) {
			return false;
		}

		mCharacteristicOta = openWriteChar(SPS_OTA_UUID);

		return true;
	}
}
