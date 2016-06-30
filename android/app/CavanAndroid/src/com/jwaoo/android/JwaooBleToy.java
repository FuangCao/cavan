package com.jwaoo.android;

import java.util.UUID;

import android.bluetooth.BluetoothDevice;

import com.cavan.android.CavanBleChar;
import com.cavan.android.CavanBleGatt;

public class JwaooBleToy extends CavanBleGatt {

	public static final UUID UUID_SERVICE = UUID.fromString("00001888-0000-1000-8000-00805f9b34fb");
	public static final UUID UUID_TX = UUID.fromString("00001889-0000-1000-8000-00805f9b34fb");
	public static final UUID UUID_RX = UUID.fromString("0000188a-0000-1000-8000-00805f9b34fb");
	public static final UUID UUID_OTA = UUID.fromString("0000188b-0000-1000-8000-00805f9b34fb");

	private CavanBleChar mCharacteristicTx;
	private CavanBleChar mCharacteristicRx;
	private CavanBleChar mCharacteristicOta;

	public JwaooBleToy(BluetoothDevice device, UUID uuid) {
		super(device, uuid);
	}

	public JwaooBleToy(BluetoothDevice device) {
		this(device, UUID_SERVICE);
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
		mCharacteristicRx = openWriteChar(UUID_RX);
		if (mCharacteristicRx == null) {
			return false;
		}

		mCharacteristicTx = openReadChar(UUID_TX);
		if (mCharacteristicTx == null) {
			return false;
		}

		mCharacteristicOta = openWriteChar(UUID_OTA);
		if (mCharacteristicOta == null) {
			return false;
		}

		return true;
	}
}
