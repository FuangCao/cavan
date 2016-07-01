package com.jwaoo.android;

import java.util.UUID;

import android.bluetooth.BluetoothDevice;
import android.content.Context;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanBleGatt;
import com.cavan.java.ByteCache;
import com.cavan.java.CavanHexFile;

public abstract class JwaooBleToy extends CavanBleGatt {

	public static final long COMMAND_TIMEOUT = 20000;
	public static final  long DATA_TIMEOUT = 5000;
	public static final UUID UUID_SERVICE = UUID.fromString("00001888-0000-1000-8000-00805f9b34fb");
	public static final UUID UUID_COMMAND = UUID.fromString("00001889-0000-1000-8000-00805f9b34fb");
	public static final UUID UUID_FLASH = UUID.fromString("0000188a-0000-1000-8000-00805f9b34fb");
	public static final UUID UUID_SENSOR = UUID.fromString("0000188b-0000-1000-8000-00805f9b34fb");

	public static final byte JWAOO_TOY_CMD_NONE = 0;
	public static final byte JWAOO_TOY_RSP_BOOL = 1;
	public static final byte JWAOO_TOY_RSP_U8 = 2;
	public static final byte JWAOO_TOY_RSP_U16 = 3;
	public static final byte JWAOO_TOY_RSP_U32 = 4;
	public static final byte JWAOO_TOY_RSP_DATA = 5;
	public static final byte JWAOO_TOY_CMD_FLASH_WEN = 6;
	public static final byte JWAOO_TOY_CMD_FLASH_READ = 7;
	public static final byte JWAOO_TOY_CMD_FLASH_SEEK = 8;
	public static final byte JWAOO_TOY_CMD_FLASH_ERASE = 9;
	public static final byte JWAOO_TOY_CMD_FLASH_WRITE_OK = 10;
	public static final byte JWAOO_TOY_CMD_SENSOR_ENABLE = 11;
	public static final byte JWAOO_TOY_CMD_SENSOR_SET_DELAY = 12;

	private CavanBleChar mCharCommand;
	private CavanBleChar mCharFlash;
	private CavanBleChar mCharSensor;

	protected abstract void onSensorDataReceived(byte[] data);

	private CavanBleDataListener mSensorListener = new CavanBleDataListener() {

		@Override
		public void onDataReceived(byte[] data) {
			onSensorDataReceived(data);
		}
	};

	public JwaooBleToy(Context context, BluetoothDevice device, UUID uuid) throws Exception {
		super(context, device, uuid);
	}

	public JwaooBleToy(Context context, BluetoothDevice device) throws Exception {
		this(context, device, UUID_SERVICE);
	}

	public byte[] sendCommand(byte[] data, long timeout) {
		if (mCharCommand == null) {
			return null;
		}

		if (!mCharCommand.writeData(data, true)) {
			return null;
		}

		return mCharCommand.readData(timeout);
	}

	public boolean sendCommandSimple(byte[] data) {
		byte[] response = sendCommand(data, COMMAND_TIMEOUT);
		if (response == null || response.length != 2 || response[0] != JWAOO_TOY_RSP_BOOL) {
			return false;
		}

		return response[1] > 0;
	}

	public boolean sendCommand(byte type) {
		return sendCommandSimple(new byte[] { type });
	}

	public boolean sendCommandBool(byte type, boolean value) {
		return sendCommandSimple(new byte[] { type, (byte) (value ? 1 : 0) });
	}

	public boolean sendCommandByte(byte type, byte value) {
		return sendCommandSimple(new byte[] { type, value });
	}

	public boolean sendCommandShort(byte type, short value) {
		ByteCache cache = new ByteCache(3);

		cache.writeValue8(type);
		cache.writeValue16(value);

		return sendCommandSimple(cache.getBytes());
	}

	public boolean sendCommandInt(byte type, int value) {
		ByteCache cache = new ByteCache(5);

		cache.writeValue8(type);
		cache.writeValue32(value);

		return sendCommandSimple(cache.getBytes());
	}

	public byte[] readFlash(int address) {
		if (!sendCommandInt(JWAOO_TOY_CMD_FLASH_READ, address)) {
			return null;
		}

		return mCharFlash.readData(DATA_TIMEOUT);
	}

	public boolean setFlashWriteEnable(boolean enable) {
		return sendCommandBool(JWAOO_TOY_CMD_FLASH_WEN, enable);
	}

	public boolean eraseFlash() {
		return sendCommand(JWAOO_TOY_CMD_FLASH_ERASE);
	}

	public boolean seekFlash(int address) {
		return sendCommandInt(JWAOO_TOY_CMD_FLASH_SEEK, address);
	}

	public boolean writeFlash(byte[] data) {
		return mCharFlash.writeData(data, true);
	}

	public boolean isFlashWriteOk() {
		return sendCommand(JWAOO_TOY_CMD_FLASH_WRITE_OK);
	}

	public boolean doOtaUpgrade(String pathname) {
		CavanHexFile file = new CavanHexFile(pathname);
		byte[] bytes = file.parse();
		if (bytes == null) {
			CavanAndroid.logE("Failed to parse hex file");
			return false;
		}

		CavanAndroid.logE("setFlashWriteEnable");

		if (!setFlashWriteEnable(true)) {
			CavanAndroid.logE("Failed to setFlashWriteEnable");
			return false;
		}

		CavanAndroid.logE("eraseFlash");

		if (!eraseFlash()) {
			CavanAndroid.logE("Failed to setFlashWriteEnable");
			return false;
		}

		CavanAndroid.logE("writeFlash header");

		int length = (bytes.length + 7) & (~0x07);
		byte[] header = { 0x70, 0x50, 0x00, 0x00, 0x00, 0x00, (byte) ((length >> 8) & 0xFF), (byte) (length & 0xFF) };
		if (!writeFlash(header)) {
			CavanAndroid.logE("Failed to writeFlash header");
			return false;
		}

		CavanAndroid.logE("writeFlash body");

		if (!writeFlash(bytes)) {
			CavanAndroid.logE("Failed to writeFlash body");
			return false;
		}

		if (!isFlashWriteOk()) {
			CavanAndroid.logE("Failed to isFlashWriteOk");
			return false;
		}

		setFlashWriteEnable(false);

		return true;
	}

	@Override
	protected boolean doInit() {
		mCharCommand = openChar(UUID_COMMAND);
		if (mCharCommand == null) {
			return false;
		}

		mCharFlash = openChar(UUID_FLASH);
		if (mCharFlash == null) {
			return false;
		}

		mCharSensor = openChar(UUID_SENSOR);
		if (mCharSensor == null) {
			return false;
		}

		mCharSensor.setDataListener(mSensorListener);

		return true;
	}
}
