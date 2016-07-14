package com.jwaoo.android;

import java.util.UUID;

import android.bluetooth.BluetoothDevice;
import android.content.Context;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanBleGatt;
import com.cavan.java.CavanByteCache;
import com.cavan.java.CavanHexFile;
import com.cavan.java.CavanJava;
import com.cavan.java.CavanProgressListener;

public abstract class JwaooBleToy extends CavanBleGatt {

	public static final  long DATA_TIMEOUT = 5000;
	public static final String IDENTIFY = "JwaooToy";
	public static final UUID UUID_SERVICE = UUID.fromString("00001888-0000-1000-8000-00805f9b34fb");
	public static final UUID UUID_COMMAND = UUID.fromString("00001889-0000-1000-8000-00805f9b34fb");
	public static final UUID UUID_EVENT = UUID.fromString("0000188a-0000-1000-8000-00805f9b34fb");
	public static final UUID UUID_FLASH = UUID.fromString("0000188b-0000-1000-8000-00805f9b34fb");
	public static final UUID UUID_SENSOR = UUID.fromString("0000188c-0000-1000-8000-00805f9b34fb");

	public static final byte JWAOO_TOY_RSP_BOOL = 0;
	public static final byte JWAOO_TOY_RSP_U8 = 1;
	public static final byte JWAOO_TOY_RSP_U16 = 2;
	public static final byte JWAOO_TOY_RSP_U32 = 3;
	public static final byte JWAOO_TOY_RSP_DATA = 4;
	public static final byte JWAOO_TOY_RSP_TEXT = 5;
	public static final byte JWAOO_TOY_CMD_NOOP = 20;
	public static final byte JWAOO_TOY_CMD_IDENTIFY = 21;
	public static final byte JWAOO_TOY_CMD_VERSION = 22;
	public static final byte JWAOO_TOY_CMD_BUILD_DATE = 23;
	public static final byte JWAOO_TOY_CMD_REBOOT = 24;
	public static final byte JWAOO_TOY_CMD_SHUTDOWN = 25;
	public static final byte JWAOO_TOY_CMD_BATT_INFO = 26;
	public static final byte JWAOO_TOY_CMD_FIND = 27;
	public static final byte JWAOO_TOY_CMD_FLASH_ID = 50;
	public static final byte JWAOO_TOY_CMD_FLASH_SIZE = 51;
	public static final byte JWAOO_TOY_CMD_FLASH_PAGE_SIZE = 52;
	public static final byte JWAOO_TOY_CMD_FLASH_READ = 53;
	public static final byte JWAOO_TOY_CMD_FLASH_SEEK = 54;
	public static final byte JWAOO_TOY_CMD_FLASH_ERASE = 55;
	public static final byte JWAOO_TOY_CMD_FLASH_WRITE_ENABLE = 56;
	public static final byte JWAOO_TOY_CMD_FLASH_WRITE_START = 57;
	public static final byte JWAOO_TOY_CMD_FLASH_WRITE_FINISH = 58;
	public static final byte JWAOO_TOY_CMD_SENSOR_ENABLE = 70;
	public static final byte JWAOO_TOY_CMD_SENSOR_SET_DELAY = 71;
	public static final byte JWAOO_TOY_CMD_MOTO_ENABLE = 80;
	public static final byte JWAOO_TOY_CMD_MOTO_SET_LEVEL = 81;

	public static final byte JWAOO_TOY_EVT_BATT_INFO = 0;
	public static final byte JWAOO_TOY_EVT_FLASH_ERROR = 1;

	private CavanBleChar mCharCommand;
	private CavanBleChar mCharEvent;
	private CavanBleChar mCharFlash;
	private CavanBleChar mCharSensor;

	protected abstract void onSensorDataReceived(byte[] data);

	private CavanBleDataListener mSensorListener = new CavanBleDataListener() {

		@Override
		public void onDataReceived(byte[] data) {
			onSensorDataReceived(data);
		}
	};

	public JwaooBleToy(Context context, BluetoothDevice device, UUID uuid) {
		super(context, device, uuid);
	}

	public JwaooBleToy(Context context, BluetoothDevice device) {
		this(context, device, UUID_SERVICE);
	}

	synchronized public byte[] sendCommand(byte[] command) {
		if (mCharCommand == null) {
			return null;
		}

		byte[] response = mCharCommand.sendCommand(command);
		if (response == null || response.length < 1) {
			CavanAndroid.logE("Failed to mCharCommand.sendCommand");
			return null;
		}

		CavanAndroid.logE("response: length = " + response.length + ", type = " + response[0]);

		return response;
	}

	public byte[] buildCommand(byte type, byte[] data) {
		if (data != null && data.length > 0) {
			byte[] command = new byte[data.length + 1];

			command[0] = type;
			CavanJava.ArrayCopy(data, 0, command, 1, data.length);

			return command;

		} else {
			return new byte[] { type };
		}
	}

	public byte[] sendCommand(byte type, byte[] data) {
		return sendCommand(buildCommand(type, data));
	}

	public boolean sendCommandReadBool(byte[] command) {
		byte[] response = sendCommand(command);
		if (response == null || response.length != 2 || response[0] != JWAOO_TOY_RSP_BOOL) {
			return false;
		}

		return response[1] > 0;
	}

	public boolean sendCommandReadBool(byte type, byte[] data) {
		return sendCommandReadBool(buildCommand(type, data));
	}

	public byte sendCommandReadByte(byte type, byte defValue) {
		byte[] response = sendCommand(type, null);
		if (response == null || response.length != 2 || response[0] != JWAOO_TOY_RSP_U8) {
			return defValue;
		}

		return response[1];
	}

	public short sendCommandReadShort(byte type, short defValue) {
		byte[] response = sendCommand(type, null);
		if (response == null || response.length != 3 || response[0] != JWAOO_TOY_RSP_U16) {
			return defValue;
		}

		return CavanJava.buildValue16(response, 1);
	}

	public int sendCommandReadInt(byte type, int defValue) {
		byte[] response = sendCommand(type, null);
		if (response == null || response.length != 5 || response[0] != JWAOO_TOY_RSP_U32) {
			return defValue;
		}

		return CavanJava.buildValue32(response, 1);
	}

	public String sendCommandReadText(byte[] command) {
		byte[] response = sendCommand(command);
		if (response == null || response.length < 2 || response[0] != JWAOO_TOY_RSP_TEXT) {
			return null;
		}

		return new String(response, 1, response.length - 1);
	}

	public String sendCommandReadText(byte type, byte[] data) {
		return sendCommandReadText(buildCommand(type, data));
	}

	public boolean sendCommandBool(byte type, boolean value) {
		return sendCommandReadBool(new byte[] { type, (byte) (value ? 1 : 0) });
	}

	public boolean sendCommandByte(byte type, byte value) {
		return sendCommandReadBool(new byte[] { type, value });
	}

	public boolean sendCommandShort(byte type, short value) {
		CavanByteCache cache = new CavanByteCache(3);

		cache.writeValue8(type);
		cache.writeValue16(value);

		return sendCommandReadBool(cache.getBytes());
	}

	public boolean sendCommandInt(byte type, int value) {
		CavanByteCache cache = new CavanByteCache(5);

		cache.writeValue8(type);
		cache.writeValue32(value);

		return sendCommandReadBool(cache.getBytes());
	}

	public String doIdentify() {
		return sendCommandReadText(JWAOO_TOY_CMD_IDENTIFY, null);
	}

	public String readBuildDate() {
		return sendCommandReadText(JWAOO_TOY_CMD_BUILD_DATE, null);
	}

	public int readVersion() {
		return sendCommandReadInt(JWAOO_TOY_CMD_VERSION, 0);
	}

	synchronized public byte[] readFlash(int address) {
		if (mCharFlash == null) {
			return null;
		}

		if (!sendCommandInt(JWAOO_TOY_CMD_FLASH_READ, address)) {
			return null;
		}

		return mCharFlash.readData(DATA_TIMEOUT);
	}

	public int getFlashId() {
		return sendCommandReadInt(JWAOO_TOY_CMD_FLASH_ID, 0);
	}

	public int getFlashSize() {
		return sendCommandReadInt(JWAOO_TOY_CMD_FLASH_SIZE, 0);
	}

	public int getFlashPageSize() {
		return sendCommandReadInt(JWAOO_TOY_CMD_FLASH_PAGE_SIZE, 0);
	}

	public boolean setFlashWriteEnable(boolean enable) {
		return sendCommandBool(JWAOO_TOY_CMD_FLASH_WRITE_ENABLE, enable);
	}

	public boolean eraseFlash() {
		return sendCommandReadBool(JWAOO_TOY_CMD_FLASH_ERASE, null);
	}

	public boolean seekFlash(int address) {
		return sendCommandInt(JWAOO_TOY_CMD_FLASH_SEEK, address);
	}

	public boolean startFlashWrite() {
		return sendCommandReadBool(JWAOO_TOY_CMD_FLASH_WRITE_START, null);
	}

	synchronized public boolean finishWriteFlash() {
		for (int i = 0; i < 10; i++) {
			if (sendCommandReadBool(JWAOO_TOY_CMD_FLASH_WRITE_FINISH, null)) {
				return true;
			}

			if (mCharCommand.isNotTimeout()) {
				break;
			}
		}

		return false;
	}

	synchronized public boolean writeFlash(byte[] data) {
		if (mCharFlash == null) {
			return false;
		}

		return mCharFlash.writeData(data, true);
	}

	synchronized public boolean writeFlash(byte[] data, CavanProgressListener listener) {
		if (mCharFlash == null) {
			return false;
		}

		return mCharFlash.writeData(data, listener);
	}

	synchronized public boolean doOtaUpgrade(String pathname, CavanProgressListener listener) {
		listener.setProgressRange(0, 99);;
		listener.startProgress();

		CavanHexFile file = new CavanHexFile(pathname);
		byte[] bytes = file.parse();
		if (bytes == null) {
			CavanAndroid.logE("Failed to parse hex file");
			return false;
		}

		CavanAndroid.logE("Flash id = " + Integer.toHexString(getFlashId()));
		CavanAndroid.logE("Flash size = " + getFlashSize());
		CavanAndroid.logE("Flash page size = " + getFlashPageSize());

		CavanAndroid.logE("setFlashWriteEnable");

		listener.addProgress();

		if (!setFlashWriteEnable(true)) {
			CavanAndroid.logE("Failed to setFlashWriteEnable true");
			return false;
		}

		listener.addProgress();

		CavanAndroid.logE("startFlashWrite");

		if (!startFlashWrite()) {
			CavanAndroid.logE("Failed to startFlashWrite");
			return false;
		}

		listener.addProgress();

		CavanAndroid.logE("eraseFlash");

		if (!eraseFlash()) {
			CavanAndroid.logE("Failed to eraseFlash");
			return false;
		}

		listener.addProgress();

		CavanAndroid.logE("writeFlash header");

		int length = (bytes.length + 7) & (~0x07);
		byte[] header = { 0x70, 0x50, 0x00, 0x00, 0x00, 0x00, (byte) ((length >> 8) & 0xFF), (byte) (length & 0xFF) };
		if (!writeFlash(header)) {
			CavanAndroid.logE("Failed to writeFlash header");
			return false;
		}

		listener.addProgress();

		CavanAndroid.logE("writeFlash body");

		if (!writeFlash(bytes, listener)) {
			CavanAndroid.logE("Failed to writeFlash body");
			return false;
		}

		CavanAndroid.logE("finishWriteFlash");

		if (!finishWriteFlash()) {
			CavanAndroid.logE("Failed to finishWriteFlash");
			return false;
		}

		listener.setProgressMax(100);
		listener.finishProgress();

		return true;
	}

	public boolean setSensorEnable(boolean enable) {
		return sendCommandBool(JWAOO_TOY_CMD_SENSOR_ENABLE, enable);
	}

	public boolean setSensorDelay(int delay) {
		return sendCommandInt(JWAOO_TOY_CMD_SENSOR_SET_DELAY, delay);
	}

	public boolean doReboot() {
		return sendCommandReadBool(JWAOO_TOY_CMD_REBOOT, null);
	}

	@Override
	protected boolean doInitialize() {
		mCharCommand = openChar(UUID_COMMAND);
		if (mCharCommand == null) {
			CavanAndroid.logE("uuid not found: " + UUID_COMMAND);
			return false;
		}

		mCharEvent = openChar(UUID_EVENT);
		if (mCharEvent == null) {
			CavanAndroid.logE("uuid not found: " + UUID_EVENT);
			return false;
		}

		mCharFlash = openChar(UUID_FLASH);
		if (mCharFlash == null) {
			CavanAndroid.logE("uuid not found: " + UUID_FLASH);
			return false;
		}

		mCharSensor = openChar(UUID_SENSOR);
		if (mCharSensor == null) {
			CavanAndroid.logE("uuid not found: " + UUID_SENSOR);
			return false;
		}

		mCharSensor.setDataListener(mSensorListener);
		setAutoConnectAllow(true);

		String identify = doIdentify();
		if (identify == null) {
			CavanAndroid.logE("Failed to doIdentify");
			return false;
		}

		CavanAndroid.logE("identify = " + identify);

		if (!IDENTIFY.equals(identify)) {
			CavanAndroid.logE("Invalid identify");
			return false;
		}

		return true;
	}
}
