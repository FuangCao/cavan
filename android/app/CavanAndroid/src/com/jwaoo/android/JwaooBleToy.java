package com.jwaoo.android;

import java.util.UUID;

import android.bluetooth.BluetoothDevice;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanBleGatt;
import com.cavan.java.CavanByteCache;
import com.cavan.java.CavanHexFile;
import com.cavan.java.CavanJava;
import com.cavan.java.CavanProgressListener;
import com.cavan.java.CavanString;

public class JwaooBleToy extends CavanBleGatt {

	public static final int MOTO_MODE_MAX = 6;
	public static final int MOTO_LEVEL_MAX = 18;
	public static final int SENSOR_DATA_SKIP = 20;
	public static final long DATA_TIMEOUT = 5000;
	public static final long JWAOO_TOY_TIME_MIN = 1000;
	public static final long JWAOO_TOY_TIME_MAX_VALUE = 3000;
	public static final long JWAOO_TOY_TIME_MAX_FREQ = 5000;
	public static final double JWAOO_TOY_ACCEL_VALUE_FUZZ = 2.0;
	public static final double JWAOO_TOY_DEPTH_VALUE_FUZZ = 6.0;

	public static final String DEVICE_NAME_COMMON = "JwaooToy";
	public static final String DEVICE_NAME_K100 = "K100";
	public static final String DEVICE_NAME_K101 = "K101";
	public static final String DEVICE_NAME_MODEL6 = "MODEL6";

	public static final int DEVICE_ID_K100 = 100;
	public static final int DEVICE_ID_K101 = 101;
	public static final int DEVICE_ID_MODEL6 = 102;

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

	public static final byte JWAOO_TOY_CMD_NOOP = 0;
	public static final byte JWAOO_TOY_CMD_IDENTIFY = 1;
	public static final byte JWAOO_TOY_CMD_VERSION = 2;
	public static final byte JWAOO_TOY_CMD_BUILD_DATE = 3;
	public static final byte JWAOO_TOY_CMD_REBOOT = 4;
	public static final byte JWAOO_TOY_CMD_SHUTDOWN = 5;
	public static final byte JWAOO_TOY_CMD_I2C_RW = 6;
	public static final byte JWAOO_TOY_CMD_FLASH_ID = 30;
	public static final byte JWAOO_TOY_CMD_FLASH_SIZE = 31;
	public static final byte JWAOO_TOY_CMD_FLASH_PAGE_SIZE = 32;
	public static final byte JWAOO_TOY_CMD_FLASH_READ = 33;
	public static final byte JWAOO_TOY_CMD_FLASH_SEEK = 34;
	public static final byte JWAOO_TOY_CMD_FLASH_ERASE = 35;
	public static final byte JWAOO_TOY_CMD_FLASH_STATE = 36;
	public static final byte JWAOO_TOY_CMD_FLASH_WRITE_ENABLE = 37;
	public static final byte JWAOO_TOY_CMD_FLASH_WRITE_START = 38;
	public static final byte JWAOO_TOY_CMD_FLASH_WRITE_FINISH = 39;
	public static final byte JWAOO_TOY_CMD_FLASH_READ_BD_ADDR = 40;
	public static final byte JWAOO_TOY_CMD_FLASH_WRITE_BD_ADDR = 41;
	public static final byte JWAOO_TOY_CMD_FACTORY_ENABLE = 50;
	public static final byte JWAOO_TOY_CMD_LED_ENABLE = 51;
	public static final byte JWAOO_TOY_CMD_READ_TEST_RESULT = 52;
	public static final byte JWAOO_TOY_CMD_WRITE_TEST_RESULT = 53;
	public static final byte JWAOO_TOY_CMD_BATT_INFO = 60;
	public static final byte JWAOO_TOY_CMD_BATT_EVENT_ENABLE = 61;
	public static final byte JWAOO_TOY_CMD_SENSOR_ENABLE = 70;
	public static final byte JWAOO_TOY_CMD_MOTO_SET_MODE = 80;
	public static final byte JWAOO_TOY_CMD_MOTO_GET_MODE = 81;
	public static final byte JWAOO_TOY_CMD_MOTO_EVENT_ENABLE = 82;
	public static final byte JWAOO_TOY_CMD_KEY_CLICK_ENABLE = 90;
	public static final byte JWAOO_TOY_CMD_KEY_LONG_CLICK_ENABLE = 91;
	public static final byte JWAOO_TOY_CMD_KEY_MULTI_CLICK_ENABLE = 92;
	public static final byte JWAOO_TOY_CMD_GPIO_GET = 100;
	public static final byte JWAOO_TOY_CMD_GPIO_SET = 101;
	public static final byte JWAOO_TOY_CMD_GPIO_CFG = 102;

	public static final byte JWAOO_TOY_EVT_BATT_INFO = 1;
	public static final byte JWAOO_TOY_EVT_KEY_STATE = 2;
	public static final byte JWAOO_TOY_EVT_KEY_CLICK = 3;
	public static final byte JWAOO_TOY_EVT_KEY_LONG_CLICK = 4;
	public static final byte JWAOO_TOY_EVT_UPGRADE_COMPLETE = 5;
	public static final byte JWAOO_TOY_EVT_MOTO_STATE_CHANGED = 6;

	private int mDepthSteps;
	private byte mFlashCrc;
	private int mDeviceId;
	private String mDeviceName;

	protected CavanBleChar mCharCommand;
	protected CavanBleChar mCharEvent;
	protected CavanBleChar mCharFlash;
	protected CavanBleChar mCharSensor;
	protected JwaooToyCommand mCommand = new JwaooToyCommand();

	protected int mSensorDataSkip;
	protected JwaooToySensor mSensor;
	protected JwaooToyParser mParser = new JwaooToyParser(JWAOO_TOY_DEPTH_VALUE_FUZZ);

	private CavanBleDataListener mEventListener = new CavanBleDataListener() {

		@Override
		public void onDataReceived(byte[] data) {
			onEventReceived(data);
		}
	};

	private CavanBleDataListener mSensorListener = new CavanBleDataListener() {

		@Override
		public void onDataReceived(byte[] data) {
			if (mSensorDataSkip > 0) {
				mSensorDataSkip--;
			} else if (mSensor != null) {
				onSensorDataReceived(data);
			}
		}
	};

	protected void onBatteryStateChanged(int state, int level, double voltage) {
		CavanAndroid.eLog("state = " + state + ", level = " + level + ", voltage = " + voltage);
	}

	protected void onKeyStateChanged(int code, int state) {
		CavanAndroid.eLog("onKeyStateChanged: code = " + code + ", state = " + state);
	}

	protected void onKeyClicked(int code, int count) {
		CavanAndroid.eLog("onKeyClicked: code = " + code + ", count = " + count);
	}

	protected void onKeyLongClicked(int code) {
		CavanAndroid.eLog("onKeyLongClicked: code = " + code);
	}

	protected void onMotoStateChanged(int mode, int level) {
		CavanAndroid.eLog("onMotoStateChanged: mode = " + mode + ", level = " + level);
	}

	protected void onUpgradeComplete(boolean success) {
		CavanAndroid.eLog("onUpgradeComplete: success = " + success);
	}

	protected void onEventReceived(byte[] event) {
		if (event.length > 0) {
			switch (event[0]) {
			case JWAOO_TOY_EVT_BATT_INFO:
				if (event.length == 5) {
					CavanByteCache cache = new CavanByteCache(event);
					cache.setOffset(1);
					onBatteryStateChanged(cache.readValue8(), cache.readValue8(), ((double) cache.readValue16()) / 1000);
				}
				break;

			case JWAOO_TOY_EVT_KEY_STATE:
				if (event.length > 2) {
					onKeyStateChanged(event[1], event[2]);
				}
				break;

			case JWAOO_TOY_EVT_KEY_CLICK:
				if (event.length > 1) {
					int count;

					if (event.length > 2) {
						count = event[2];
					} else {
						count = 1;
					}

					onKeyClicked(event[1], count);
				}
				break;

			case JWAOO_TOY_EVT_KEY_LONG_CLICK:
				if (event.length > 1) {
					onKeyLongClicked(event[1]);
				}
				break;

			case JWAOO_TOY_EVT_UPGRADE_COMPLETE:
				if (event.length == 2) {
					onUpgradeComplete(event[1] > 0);
				}
				break;

			case JWAOO_TOY_EVT_MOTO_STATE_CHANGED:
				if (event.length == 3) {
					onMotoStateChanged(event[1], event[2]);
				}
				break;

			default:
				CavanAndroid.eLog("unknown event" + event[0]);
			}
		}
	}

	protected void onSensorDataReceived(byte[] data) {
		mSensor.putBytes(data);
		mParser.putData(mSensor);
	}

	protected void onDebugDataReceived(byte[] data) {
		CavanAndroid.eLog("Debug: " + new String(data));
	}

	public JwaooBleToy(BluetoothDevice device, UUID uuid) {
		super(device, uuid);
	}

	public JwaooBleToy(BluetoothDevice device) {
		this(device, UUID_SERVICE);
	}

	public JwaooToySensor getSensor() {
		return mSensor;
	}

	public void setSensor(JwaooToySensor sensor) {
		if (sensor != null) {
			mSensor = sensor;
		}
	}

	public void setFreqFuzz(double fuzz) {
		mParser.setFreqFuzz(fuzz);
	}

	public void setDepthFuzz(double fuzz) {
		mParser.setDepthFuzz(fuzz);
	}

	public void setTimeMin(long time) {
		mParser.setTimeMin(time);
	}

	public void setTimeMax(long time) {
		mParser.setTimeMax(time);
	}

	public double getDepth() {
		return mParser.getDepth();
	}

	public int getDepthRaw() {
		return mParser.getDepthRaw();
	}

	public double getFreq() {
		return mParser.getFreq();
	}

	public boolean isCommandTimeout() {
		return mCharCommand == null || mCharCommand.isTimeout();
	}

	public String doIdentify() {
		return mCommand.readText(JWAOO_TOY_CMD_IDENTIFY);
	}

	public String readBuildDate() {
		return mCommand.readText(JWAOO_TOY_CMD_BUILD_DATE);
	}

	public int readVersion() {
		return mCommand.readValue32(JWAOO_TOY_CMD_VERSION, -1);
	}

	synchronized public byte[] readFlash(int address) {
		if (mCharFlash == null) {
			return null;
		}

		if (!mCommand.readBool(JWAOO_TOY_CMD_FLASH_READ, address)) {
			return null;
		}

		return mCharFlash.readData(DATA_TIMEOUT);
	}

	public int getFlashId() {
		return mCommand.readValue32(JWAOO_TOY_CMD_FLASH_ID, -1);
	}

	public int getFlashSize() {
		return mCommand.readValue32(JWAOO_TOY_CMD_FLASH_SIZE, -1);
	}

	public int getFlashPageSize() {
		return mCommand.readValue32(JWAOO_TOY_CMD_FLASH_PAGE_SIZE, -1);
	}

	public boolean setFlashWriteEnable(boolean enable) {
		return mCommand.readBool(JWAOO_TOY_CMD_FLASH_WRITE_ENABLE, enable);
	}

	public boolean eraseFlash() {
		return mCommand.readBool(JWAOO_TOY_CMD_FLASH_ERASE);
	}

	public boolean seekFlash(int address) {
		return mCommand.readBool(JWAOO_TOY_CMD_FLASH_SEEK, address);
	}

	public boolean startFlashWrite() {
		return mCommand.readBool(JWAOO_TOY_CMD_FLASH_WRITE_START);
	}

	synchronized public boolean finishWriteFlash(int length) {
		byte[] command = { JWAOO_TOY_CMD_FLASH_WRITE_FINISH, mFlashCrc, (byte) (length & 0xFF), (byte) ((length >> 8) & 0xFF) };

		for (int i = 0; i < 10; i++) {
			if (mCommand.readBool(command)) {
				return true;
			}

			if (mCharCommand.isNotTimeout()) {
				break;
			}
		}

		return false;
	}

	synchronized public boolean writeFlash(byte[] data, CavanProgressListener listener) {
		if (mCharFlash == null) {
			return false;
		}

		if (!mCharFlash.writeData(data, listener)) {
			return false;
		}

		for (byte value : data) {
			mFlashCrc ^= value;
		}

		return true;
	}

	synchronized private boolean writeFlashHeader(int length) {
		length = (length + 7) & (~0x07);

		byte[] header = { 0x70, 0x50, 0x00, 0x00, 0x00, 0x00, (byte) ((length >> 8) & 0xFF), (byte) (length & 0xFF) };

		return writeFlash(header, null);
	}

	synchronized public boolean doOtaUpgrade(String pathname, CavanProgressListener listener) {
		listener.setProgressRange(0, 99);;
		listener.startProgress();

		CavanHexFile file = new CavanHexFile(pathname);
		byte[] bytes = file.parse();
		if (bytes == null) {
			CavanAndroid.eLog("Failed to parse hex file");
			return false;
		}

		CavanAndroid.eLog("Firmware size = " + bytes.length);

		CavanAndroid.eLog("Flash id = " + Integer.toHexString(getFlashId()));
		CavanAndroid.eLog("Flash size = " + getFlashSize());
		CavanAndroid.eLog("Flash page size = " + getFlashPageSize());

		CavanAndroid.eLog("setFlashWriteEnable");

		listener.addProgress();

		if (!setFlashWriteEnable(true)) {
			CavanAndroid.eLog("Failed to setFlashWriteEnable true");
			return false;
		}

		listener.addProgress();

		CavanAndroid.eLog("startFlashWrite");

		if (!startFlashWrite()) {
			CavanAndroid.eLog("Failed to startFlashWrite");
			return false;
		}

		listener.addProgress();

		CavanAndroid.eLog("eraseFlash");

		if (!eraseFlash()) {
			CavanAndroid.eLog("Failed to eraseFlash");
			return false;
		}

		listener.addProgress();

		mFlashCrc = (byte) 0xFF;

		CavanAndroid.eLog("writeFlashHeader");

		if (!writeFlashHeader(bytes.length)) {
			CavanAndroid.eLog("Failed to writeFlashHeader");
			return false;
		}

		listener.addProgress();

		CavanAndroid.eLog("writeFlash body");

		if (!writeFlash(bytes, listener)) {
			CavanAndroid.eLog("Failed to writeFlash body");
			return false;
		}

		CavanAndroid.eLog("finishWriteFlash");

		if (!finishWriteFlash(bytes.length + 8)) {
			CavanAndroid.eLog("Failed to finishWriteFlash");
			return false;
		}

		for (int i = 0; i < 10; i++) {
			CavanAndroid.eLog("wait upgrade complete" + i);

			if (setFlashWriteEnable(false)) {
				listener.setProgressMax(100);
				listener.finishProgress();
				return true;
			}

			if (mCharCommand.getWriteStatus() == -110) {
				return false;
			}

			try {
				wait(500);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}

		return false;
	}

	public boolean setSensorEnable(boolean enable) {
		mSensorDataSkip = SENSOR_DATA_SKIP;

		return mCommand.readBool(JWAOO_TOY_CMD_SENSOR_ENABLE, enable);
	}

	public boolean setSensorEnable(boolean enable, int delay) {
		mSensorDataSkip = SENSOR_DATA_SKIP;

		return mCommand.readBool(JWAOO_TOY_CMD_SENSOR_ENABLE, enable, delay);
	}

	public boolean setMotoMode(int mode, int level) {
		byte[] command = { JWAOO_TOY_CMD_MOTO_SET_MODE, (byte) mode, (byte) level };
		return mCommand.readBool(command);
	}

	public JwaooToyMotoMode getMotoMode() {
		JwaooToyResponse response = mCommand.send(JWAOO_TOY_CMD_MOTO_GET_MODE);
		if (response == null) {
			return null;
		}

		return response.getMotoMode();
	}

	public boolean setMotoEventEnable(boolean enable) {
		return mCommand.readBool(JWAOO_TOY_CMD_MOTO_EVENT_ENABLE, enable);
	}

	public boolean setFactoryModeEnable(boolean enable) {
		return mCommand.readBool(JWAOO_TOY_CMD_FACTORY_ENABLE, enable);
	}

	public boolean setBatteryEventEnable(boolean enable) {
		return mCommand.readBool(JWAOO_TOY_CMD_BATT_EVENT_ENABLE, enable);
	}

	public boolean doReboot() {
		return mCommand.readBool(JWAOO_TOY_CMD_REBOOT);
	}

	public boolean doShutdown() {
		return mCommand.readBool(JWAOO_TOY_CMD_SHUTDOWN);
	}

	public byte[] readBdAddress() {
		byte[] bytes = mCommand.readData(JWAOO_TOY_CMD_FLASH_READ_BD_ADDR);
		if (bytes != null && bytes.length == 6) {
			return bytes;
		}

		return null;
	}

	public String readBdAddressString() {
		byte[] bytes = readBdAddress();
		if (bytes == null) {
			return null;
		}

		return CavanString.fromBdAddr(bytes);
	}

	public boolean writeBdAddress(byte[] bytes) {
		if (!setFlashWriteEnable(true)) {
			CavanAndroid.eLog("Failed to setFlashWriteEnable true");
			return false;
		}

		if (!mCommand.readBool(JWAOO_TOY_CMD_FLASH_WRITE_BD_ADDR, bytes)) {
			return false;
		}

		return setFlashWriteEnable(false);
	}

	public boolean writeBdAddress(String addr) {
		byte[] bytes = CavanString.parseBdAddr(addr);
		if (bytes == null) {
			return false;
		}

		return writeBdAddress(bytes);
	}

	public boolean setClickEnable(boolean enable) {
		return mCommand.readBool(JWAOO_TOY_CMD_KEY_CLICK_ENABLE, enable);
	}

	public boolean setMultiClickEnable(boolean enable) {
		return mCommand.readBool(JWAOO_TOY_CMD_KEY_MULTI_CLICK_ENABLE, enable);
	}

	public boolean setMultiClickEnable(boolean enable, short delay) {
		return mCommand.readBool(JWAOO_TOY_CMD_KEY_MULTI_CLICK_ENABLE, enable, delay);
	}

	public boolean setLongClickEnable(boolean enable) {
		return mCommand.readBool(JWAOO_TOY_CMD_KEY_LONG_CLICK_ENABLE, enable);
	}

	public boolean setLongClickEnable(boolean enable, short delay) {
		return mCommand.readBool(JWAOO_TOY_CMD_KEY_LONG_CLICK_ENABLE, enable, delay);
	}

	public JwaooToyMpu6050 createMpu6050() {
		return new JwaooToyMpu6050();
	}

	public JwaooToyBmi160 createBmi160() {
		return new JwaooToyBmi160();
	}

	public JwaooToyFdc1004 createFdc1004() {
		return new JwaooToyFdc1004();
	}

	public boolean getGpioValue(int port, int pin) {
		byte[] command = { JWAOO_TOY_CMD_GPIO_GET, (byte) port, (byte) pin };
		return mCommand.readValue8(command, (byte) -1) > 0;
	}

	public boolean setGpioValue(int port, int pin, boolean value) {
		byte[] command = { JWAOO_TOY_CMD_GPIO_GET, (byte) port, (byte) pin, CavanJava.getBoolValueByte(value) };
		return mCommand.readBool(command);
	}

	public boolean doConfigGpio(int port, int pin, int mode, int function, boolean high) {
		byte[] command = { JWAOO_TOY_CMD_GPIO_CFG, (byte) port, (byte) pin, (byte) mode, (byte) function, CavanJava.getBoolValueByte(high) };
		return mCommand.readBool(command);
	}

	public boolean setLedEnable(int index, boolean enable) {
		byte[] command = { JWAOO_TOY_CMD_LED_ENABLE, (byte) index, CavanJava.getBoolValueByte(enable) };
		return mCommand.readBool(command);
	}

	public JwaooToyTestResult readTestResult() {
		JwaooToyResponse response = mCommand.send(JWAOO_TOY_CMD_READ_TEST_RESULT);
		if (response == null) {
			return null;
		}

		return response.getTestResult();
	}

	public boolean writeTestResult(JwaooToyTestResult result) {
		return mCommand.readBool(result.buildCommand());
	}

	public int getDeviveId() {
		return mDeviceId;
	}

	public String getDeviceName() {
		return mDeviceName;
	}

	public int getDepthSteps() {
		return mDepthSteps;
	}

	@Override
	protected boolean doInitialize() {
		mCharCommand = openChar(UUID_COMMAND);
		if (mCharCommand == null) {
			CavanAndroid.eLog("uuid not found: " + UUID_COMMAND);
			return false;
		}

		mCharEvent = openChar(UUID_EVENT);
		if (mCharEvent == null) {
			CavanAndroid.eLog("uuid not found: " + UUID_EVENT);
			return false;
		}

		mCharFlash = openChar(UUID_FLASH);
		if (mCharFlash == null) {
			CavanAndroid.eLog("uuid not found: " + UUID_FLASH);
			return false;
		}

		mCharSensor = openChar(UUID_SENSOR);
		if (mCharSensor == null) {
			CavanAndroid.eLog("uuid not found: " + UUID_SENSOR);
			return false;
		}

		setAutoConnectAllow(true);

		if (!mCharEvent.setDataListener(mEventListener)) {
			CavanAndroid.eLog("Failed to mCharEvent.setDataListener");
			return false;
		}

		if (!mCharSensor.setDataListener(mSensorListener)) {
			CavanAndroid.eLog("Failed to mCharSensor.setDataListener");
			return false;
		}

		setReady(true);

		String identify = doIdentify();
		if (identify == null) {
			CavanAndroid.eLog("Failed to doIdentify");
			return false;
		}

		CavanAndroid.eLog("identify = " + identify);

		if (identify.equals(DEVICE_NAME_COMMON) || identify.equals(DEVICE_NAME_K100)) {
			mSensor = new JwaooToySensorK100();
			mDeviceName = DEVICE_NAME_K100;
			mDeviceId = DEVICE_ID_K100;
			mDepthSteps = 4;
		} else if (identify.equals(DEVICE_NAME_K101)) {
			mSensor = new JwaooToySensorK101();
			mDeviceName = DEVICE_NAME_K101;
			mDeviceId = DEVICE_ID_K101;
			mDepthSteps = 3;
		} else if (identify.equals(DEVICE_NAME_MODEL6)) {
			mSensor = new JwaooToySensorK101();
			mDeviceName = DEVICE_NAME_MODEL6;
			mDeviceId = DEVICE_ID_MODEL6;
			mDepthSteps = 3;
		} else {
			CavanAndroid.eLog("Invalid identify");
			return false;
		}

		return true;
	}

	// ================================================================================

	public static class JwaooToyMotoMode {

		private int mMode;
		private int mLevel;

		public JwaooToyMotoMode(int mode, int level) {
			mMode = mode;
			mLevel = level;
		}

		public int getMode() {
			return mMode;
		}

		public void setMode(int mode) {
			mMode = mode;
		}

		public int getLevel() {
			return mLevel;
		}

		public void setLevel(int level) {
			mLevel = level;
		}

		@Override
		public String toString() {
			return "mode = " + mMode + ", level = " + mLevel;
		}
	}

	public static class JwaooToyTestResult {

		private int mTestValid;
		private int mTestResult;

		public JwaooToyTestResult(byte[] response) {
			mTestValid = CavanJava.buildValue16(response, 2);
			mTestResult = CavanJava.buildValue16(response, 4);
		}

		public byte[] buildCommand() {
			CavanByteCache cache = new CavanByteCache(5);
			cache.writeValue8(JWAOO_TOY_CMD_WRITE_TEST_RESULT);
			cache.writeValue16((short) mTestValid);
			cache.writeValue16((short) mTestResult);
			return cache.getBytes();
		}

		public int getResult(int index) {
			int mask = 1 << index;

			if ((mTestValid & mask) == 0) {
				return -1;
			}

			if ((mTestResult & mask) == 0) {
				return 0;
			}

			return 1;
		}

		public void setResult(int index, int result) {
			int mask = 1 << index;

			if (result < 0) {
				mTestValid &= ~mask;
			} else {
				mTestValid |= mask;

				if (result > 0) {
					mTestResult |= mask;
				} else {
					mTestResult &= ~mask;

				}
			}
		}

		@Override
		public String toString() {
			return String.format("valid = 0x%08x, result = 0x%08x", mTestValid, mTestResult);
		}
	}

	public static class JwaooToyResponse {

		private byte[] mBytes;

		public JwaooToyResponse(byte[] bytes) {
			mBytes = bytes;
		}

		public byte getCommand() {
			return mBytes[0];
		}

		public byte getType() {
			return mBytes[1];
		}

		public int length() {
			return mBytes.length;
		}

		public boolean getBool() {
			if (getType() != JWAOO_TOY_RSP_BOOL || length() != 3) {
				return false;
			}

			return mBytes[2] != 0;
		}

		public byte getValue8(byte defValue) {
			if (getType() != JWAOO_TOY_RSP_U8 || length() != 3) {
				return defValue;
			}

			return mBytes[2];
		}

		public short getValue16(short defValue) {
			if (getType() != JWAOO_TOY_RSP_U16 || length() != 4) {
				return defValue;
			}

			return CavanJava.buildValue16(mBytes, 2);
		}

		public int getValue32(int defValue) {
			if (getType() != JWAOO_TOY_RSP_U32 || length() != 6) {
				return defValue;
			}

			return CavanJava.buildValue32(mBytes, 2);
		}

		public String getText() {
			if (getType() != JWAOO_TOY_RSP_TEXT) {
				return null;
			}

			return new String(mBytes, 2, length() - 2);
		}

		public byte[] getData() {
			if (getType() != JWAOO_TOY_RSP_DATA) {
				return null;
			}

			return CavanJava.ArrayCopySkip(mBytes, 2);
		}

		public JwaooToyTestResult getTestResult() {
			if (getType() != JWAOO_TOY_RSP_DATA || length() != 6) {
				return null;
			}

			return new JwaooToyTestResult(mBytes);
		}

		public JwaooToyMotoMode getMotoMode() {
			if (getType() != JWAOO_TOY_RSP_DATA || length() != 4) {
				return null;
			}

			return new JwaooToyMotoMode(mBytes[2], mBytes[3]);
		}

		public static boolean getBool(JwaooToyResponse response) {
			return response != null && response.getBool();
		}

		public static byte getValue8(JwaooToyResponse response, byte defValue) {
			if (response == null) {
				return defValue;
			}

			return response.getValue8(defValue);
		}

		public static short getValue16(JwaooToyResponse response, short defValue) {
			if (response == null) {
				return defValue;
			}

			return response.getValue16(defValue);
		}

		public static int getValue32(JwaooToyResponse response, int defValue) {
			if (response == null) {
				return defValue;
			}

			return response.getValue32(defValue);
		}

		public static String getText(JwaooToyResponse response) {
			if (response == null) {
				return null;
			}

			return response.getText();
		}

		public static byte[] getData(JwaooToyResponse response) {
			if (response == null) {
				return null;
			}

			return response.getData();
		}
	}

	// ================================================================================

	public class JwaooToyCommand {

		synchronized public JwaooToyResponse send(byte[] command) {
			for (int i = 0; i < 10; i++) {
				if (mCharCommand == null) {
					break;
				}

				byte[] response = mCharCommand.sendCommand(command);
				if (response == null) {
					CavanAndroid.eLog("Failed to mCharCommand.send");
					break;
				}

				if (response.length < 2) {
					CavanAndroid.eLog("Invalid response length: " + response.length);
					continue;
				}

				if (response[0] == command[0]) {
					CavanAndroid.eLog("response: command = " + response[0] + ", type = " + response[1] + ", length = " + response.length);
					return new JwaooToyResponse(response);
				}

				CavanAndroid.eLog("Invalid response command type: " + response[0] + ", expect = " + command[0]);
			}

			return null;
		}

		public JwaooToyResponse send(byte type) {
			return send(new byte[] { type });
		}

		public byte[] buildCommand(byte type, byte[] data) {
			byte[] command = new byte[data.length + 1];

			command[0] = type;
			CavanJava.ArrayCopy(data, 0, command, 1, data.length);

			return command;
		}

		public JwaooToyResponse send(byte type, byte[] data) {
			return send(buildCommand(type, data));
		}

		public JwaooToyResponse send(byte type, String text) {
			return send(type, text.getBytes());
		}

		public JwaooToyResponse send(byte type, byte value) {
			byte[] command = { type, value };
			return send(command);
		}

		public JwaooToyResponse send(byte type, short value) {
			CavanByteCache cache = new CavanByteCache(3);

			cache.writeValue8(type);
			cache.writeValue16(value);

			return send(cache.getBytes());
		}

		public JwaooToyResponse send(byte type, int value) {
			CavanByteCache cache = new CavanByteCache(5);

			cache.writeValue8(type);
			cache.writeValue32(value);

			return send(cache.getBytes());
		}

		public JwaooToyResponse send(byte type, boolean value) {
			return send(type, CavanJava.getBoolValueByte(value));
		}

		public JwaooToyResponse send(byte type, boolean enable, byte value) {
			byte[] command = { type, CavanJava.getBoolValueByte(enable) , value };
			return send(command);
		}

		public JwaooToyResponse send(byte type, boolean enable, short value) {
			CavanByteCache cache = new CavanByteCache(4);

			cache.writeValue8(type);
			cache.writeBool(enable);
			cache.writeValue16(value);

			return send(cache.getBytes());
		}

		public JwaooToyResponse send(byte type, boolean enable, int value) {
			CavanByteCache cache = new CavanByteCache(6);

			cache.writeValue8(type);
			cache.writeBool(enable);
			cache.writeValue32(value);

			return send(cache.getBytes());
		}

		public boolean readBool(byte[] command) {
			return JwaooToyResponse.getBool(send(command));
		}

		public boolean readBool(byte type) {
			return JwaooToyResponse.getBool(send(type));
		}

		public boolean readBool(byte type, byte value) {
			return JwaooToyResponse.getBool(send(type, value));
		}

		public boolean readBool(byte type, short value) {
			return JwaooToyResponse.getBool(send(type, value));
		}

		public boolean readBool(byte type, int value) {
			return JwaooToyResponse.getBool(send(type, value));
		}

		public boolean readBool(byte type, String text) {
			return JwaooToyResponse.getBool(send(type, text));
		}

		public boolean readBool(byte type, byte[] data) {
			return JwaooToyResponse.getBool(send(type, data));
		}

		public boolean readBool(byte type, boolean enable) {
			return JwaooToyResponse.getBool(send(type, enable));
		}

		public boolean readBool(byte type, boolean enable, byte value) {
			return JwaooToyResponse.getBool(send(type, enable, value));
		}

		public boolean readBool(byte type, boolean enable, short value) {
			return JwaooToyResponse.getBool(send(type, enable, value));
		}

		public boolean readBool(byte type, boolean enable, int value) {
			return JwaooToyResponse.getBool(send(type, enable, value));
		}

		public byte readValue8(byte[] command, byte defValue) {
			return JwaooToyResponse.getValue8(send(command), defValue);
		}

		public byte readValue8(byte type, byte defValue) {
			return JwaooToyResponse.getValue8(send(type), defValue);
		}

		public short readValue16(byte type, short defValue) {
			return JwaooToyResponse.getValue16(send(type), defValue);
		}

		public int readValue32(byte type, int defValue) {
			return JwaooToyResponse.getValue32(send(type), defValue);
		}

		public String readText(byte type) {
			return JwaooToyResponse.getText(send(type));
		}

		public byte[] readData(byte[] command) {
			return JwaooToyResponse.getData(send(command));
		}

		public byte[] readData(byte type) {
			return JwaooToyResponse.getData(send(type));
		}
	}

	// ================================================================================

	public abstract class JwaooToyI2c {
		private byte mSlave;

		protected abstract int readRegister(int addr);
		protected abstract boolean writeRegister(int addr, int value);

		public JwaooToyI2c(byte slave) {
			mSlave = slave;
		}

		public byte[] doReadWrite(int rdlen, byte[] data) {
			byte[] command = new byte[data.length + 3];

			command[0] = JWAOO_TOY_CMD_I2C_RW;
			command[1] = mSlave;
			command[2] = (byte) rdlen;
			CavanJava.ArrayCopy(data, 0, command, 3, data.length);

			return mCommand.readData(command);
		}

		public byte[] readData(byte addr, int rdlen) {
			return doReadWrite(rdlen, CavanJava.getValueBytes(addr));
		}

		public byte[] readData(short addr, int rdlen) {
			return doReadWrite(rdlen, CavanJava.getValueBytes(addr));
		}

		public byte[] readData(int addr, int rdlen) {
			return doReadWrite(rdlen, CavanJava.getValueBytes(addr));
		}

		public byte readValue8(byte addr, byte defValue) {
			byte[] data = readData(addr, 1);
			if (data == null || data.length != 1) {
				return defValue;
			}

			return data[0];
		}

		public byte readValue8(byte addr) {
			return readValue8(addr, (byte) 0);
		}

		public short readValue16(byte addr, short defValue) {
			byte[] data = readData(addr, 2);
			if (data == null || data.length != 2) {
				return defValue;
			}

			return CavanJava.buildValue16(data, 0);
		}

		public short readValue16(byte addr) {
			return readValue16(addr, (short) 0);
		}

		public int readValue32(byte addr, int defValue) {
			byte[] data = readData(addr, 4);
			if (data == null || data.length != 4) {
				return defValue;
			}

			return CavanJava.buildValue32(data, 0);
		}

		public int readValue32(byte addr) {
			return readValue32(addr, 0);
		}

		public short readValueBe16(byte addr, short defValue) {
			byte[] data = readData(addr, 2);
			if (data == null || data.length != 2) {
				return defValue;
			}

			return CavanJava.buildValueBe16(data, 0);
		}

		public short readValueBe16(byte addr) {
			return readValueBe16(addr, (short) 0);
		}

		public int readValueBe32(byte addr, int defValue) {
			byte[] data = readData(addr, 4);
			if (data == null || data.length != 4) {
				return defValue;
			}

			return CavanJava.buildValueBe32(data, 0);
		}

		public int readValueBe32(byte addr) {
			return readValueBe32(addr, 0);
		}

		public boolean writeData(byte[] data) {
			return doReadWrite(0, data) != null;
		}

		public boolean writeData(byte addr, byte[] data) {
			CavanByteCache cache = new CavanByteCache(data.length + 1);
			cache.writeValue8(addr);
			cache.writeBytes(data);
			return writeData(cache.getBytes());
		}

		public boolean writeData(short addr, byte[] data) {
			CavanByteCache cache = new CavanByteCache(data.length + 2);
			cache.writeValue16(addr);
			cache.writeBytes(data);
			return writeData(cache.getBytes());
		}

		public boolean writeData(int addr, byte[] data) {
			CavanByteCache cache = new CavanByteCache(data.length + 4);
			cache.writeValue32(addr);
			cache.writeBytes(data);
			return writeData(cache.getBytes());
		}

		public boolean writeValue8(byte addr, byte value) {
			byte[] data = { addr, value };
			return writeData(data);
		}

		public boolean writeValue16(byte addr, short value) {
			CavanByteCache cache = new CavanByteCache(3);
			cache.writeValue8(addr);
			cache.writeValue16(value);
			return writeData(cache.getBytes());
		}

		public boolean writeValue32(byte addr, int value) {
			CavanByteCache cache = new CavanByteCache(5);
			cache.writeValue8(addr);
			cache.writeValue32(value);
			return writeData(cache.getBytes());
		}

		public boolean writeValueBe16(byte addr, short value) {
			CavanByteCache cache = new CavanByteCache(3);
			cache.writeValue8(addr);
			cache.writeValueBe16(value);
			return writeData(cache.getBytes());
		}

		public boolean writeValueBe32(byte addr, int value) {
			CavanByteCache cache = new CavanByteCache(5);
			cache.writeValue8(addr);
			cache.writeValueBe32(value);
			return writeData(cache.getBytes());
		}

		public int readValue8(int addr) {
			return readValue8((byte) addr) & 0xFF;
		}

		public int readValue16(int addr) {
			return readValue16((byte) addr) & 0xFFFF;
		}

		public int readValue32(int addr) {
			return readValue32((byte) addr);
		}

		public int readValueBe16(int addr) {
			return readValueBe16((byte) addr) & 0xFFFF;
		}

		public int readValueBe32(int addr) {
			return readValueBe32((byte) addr);
		}

		public boolean writeValue8(int addr, int value) {
			return writeValue8((byte) addr, (byte) value);
		}

		public boolean writeValue16(int addr, int value) {
			return writeValue16((byte) addr, (short) value);
		}

		public boolean writeValue32(int addr, int value) {
			return writeValue32((byte) addr, value);
		}

		public boolean writeValueBe16(int addr, int value) {
			return writeValueBe16((byte) addr, (short) value);
		}

		public boolean writeValueBe32(int addr, int value) {
			return writeValueBe32((byte) addr, value);
		}

		public boolean updateBits8(int addr, int value, int mask) {
			int valueOld = readValue8(addr);

			value |= (valueOld & (~mask));

			CavanAndroid.eLog(String.format("updateBits8: addr = 0x%02x, value: 0x%02x => 0x%02x", addr, valueOld, value));

			if (value == valueOld) {
				return true;
			}

			return writeValue8(addr, value);
		}

		public boolean updateBits16(int addr, int value, int mask) {
			int valueOld = readValue16(addr);

			value |= (valueOld & (~mask));

			CavanAndroid.eLog(String.format("updateBits16: addr = 0x%02x, value: 0x%04x => 0x%04x", addr, valueOld, value));

			if (value == valueOld) {
				return true;
			}

			return writeValue16(addr, value);
		}

		public boolean updateBits32(int addr, int value, int mask) {
			int valueOld = readValue32(addr);

			value |= (valueOld & (~mask));

			CavanAndroid.eLog(String.format("updateBits32: addr = 0x%02x, value: 0x%08x => 0x%08x", addr, valueOld, value));

			if (value == valueOld) {
				return true;
			}

			return writeValue32(addr, value);
		}

		public boolean updateBitsBe16(int addr, int value, int mask) {
			int valueOld = readValueBe16(addr);

			value |= (valueOld & (~mask));

			CavanAndroid.eLog(String.format("updateBitsBe16: addr = 0x%02x, value: 0x%04x => 0x%04x", addr, valueOld, value));

			if (value == valueOld) {
				return true;
			}

			return writeValueBe16(addr, value);
		}

		public boolean updateBitsBe32(int addr, int value, int mask) {
			int valueOld = readValueBe32(addr);

			value |= (valueOld & (~mask));

			CavanAndroid.eLog(String.format("updateBits32: addr = 0x%02x, value: 0x%08x => 0x%08x", addr, valueOld, value));

			if (value == valueOld) {
				return true;
			}

			return writeValueBe32(addr, value);
		}

		public boolean updateRegister(int addr, int value, int mask) {
			int valueOld = readRegister(addr);

			value |= (valueOld & (~mask));

			CavanAndroid.eLog(String.format("updateRegister: addr = 0x%02x, value: 0x%08x => 0x%08x", addr, valueOld, value));

			if (value == valueOld) {
				return true;
			}

			return writeRegister(addr, value);
		}
	}

	public abstract class JwaooToyAccelSensor extends JwaooToyI2c {

		protected double mAccelX;
		protected double mAccelY;
		protected double mAccelZ;

		public JwaooToyAccelSensor(byte slave) {
			super(slave);
		}

		public double getAccelX() {
			return mAccelX;
		}

		public double getAccelY() {
			return mAccelY;
		}

		public double getAccelZ() {
			return mAccelZ;
		}

		public void updateData8(byte[] data) {
			mAccelX = data[0] * 9.8 / 64;
			mAccelY = data[1] * 9.8 / 64;
			mAccelZ = data[2] * 9.8 / 64;
		}

		public void updateData16(byte[] data) {
			mAccelX = CavanJava.buildValue16(data, 0) * 9.8 / 16384;
			mAccelY = CavanJava.buildValue16(data, 2) * 9.8 / 16384;
			mAccelZ = CavanJava.buildValue16(data, 4) * 9.8 / 16384;
		}

		public void updateDataBe16(byte[] data) {
			mAccelX = CavanJava.buildValueBe16(data, 0) * 9.8 / 16384;
			mAccelY = CavanJava.buildValueBe16(data, 2) * 9.8 / 16384;
			mAccelZ = CavanJava.buildValueBe16(data, 4) * 9.8 / 16384;
		}

		@Override
		public String toString() {
			return String.format("[%5.2f, %5.2f, %5.2f]", mAccelX, mAccelY, mAccelZ);
		}
	}

	public class JwaooToyMpu6050 extends JwaooToyAccelSensor {

		public JwaooToyMpu6050() {
			super((byte) 0x69);
		}

		public int readChipId() {
			return readRegister(0x75);
		}

		public boolean doInitialize() {
			int id = readChipId();

			CavanAndroid.eLog(String.format("MPU6050: Chip ID = 0x%02x", id));

			if (id != 0x68) {
				CavanAndroid.eLog("Invalid chip id");
				return false;
			}

			return true;
		}

		public boolean setEnable(boolean enable) {
			return updateRegister(0x6B, (enable ? 0 : 1) << 6, 1 << 6);
		}

		public byte[] readData() {
			return readData((byte) 0x3B, 6);
		}

		public boolean updateData() {
			byte[] data = readData();
			if (data == null) {
				return false;
			}

			updateDataBe16(data);

			return true;
		}

		@Override
		protected int readRegister(int addr) {
			return readValue8(addr);
		}

		@Override
		protected boolean writeRegister(int addr, int value) {
			return writeValue8(addr, value);
		}
	}

	public class JwaooToyBmi160 extends JwaooToyAccelSensor {

		public JwaooToyBmi160() {
			super((byte) 0x68);
		}

		public int readChipId() {
			return readRegister(0x00);
		}

		public boolean doInitialize() {
			int id = readChipId();

			CavanAndroid.eLog(String.format("BMI160: Chip ID = 0x%02x", id));

			if (id != 0xd3) {
				CavanAndroid.eLog("Invalid chip id");
				return false;
			}

			return true;
		}

		public boolean setEnable(boolean enable) {
			return writeRegister(0x7E, enable ? 0x11 : 0x10);
		}

		public byte[] readData() {
			return readData((byte) 0x12, 6);
		}

		public boolean updateData() {
			byte[] data = readData();
			if (data == null) {
				return false;
			}

			updateData16(data);

			return true;
		}

		@Override
		protected int readRegister(int addr) {
			return readValue8(addr);
		}

		@Override
		protected boolean writeRegister(int addr, int value) {
			return writeValue8(addr, value);
		}
	}

	public class JwaooToyFdc1004 extends JwaooToyI2c {

		protected int[] mCapacitys = new int[4];

		public JwaooToyFdc1004() {
			super((byte) 0x50);
		}

		public int getCapacity(int index) {
			return mCapacitys[index];
		}

		public int readDeviceId() {
			return readRegister(0xFF);
		}

		public int readManufacturerId() {
			return readRegister(0xFE);
		}

		public boolean doInitialize() {
			int id;

			id = readDeviceId();
			CavanAndroid.eLog(String.format("FDC1004: Device ID = 0x%04x", id));

			if (id != 0x1004) {
				CavanAndroid.eLog("Invalid device id");
				return false;
			}

			id = readManufacturerId();
			CavanAndroid.eLog(String.format("FDC1004: Manufacturer ID = 0x%04x", id));

			if (id != 0x5449) {
				CavanAndroid.eLog("Invalid manufacturer id");
				return false;
			}

			return true;
		}

		public boolean setEnable(boolean enable) {
			int value;

			if (enable) {
				value = 3 << 10 | 1 << 8 | 0x0F << 4;
			} else {
				value = 0x0000;
			}

			return writeRegister(0x0C, value);
		}

		public boolean setOffset(int offset) {
			CavanAndroid.eLog("setOffset = " + offset);

			int value = (offset & 0x1F) << 11;

			for (int i = 0; i < 4; i++) {
				if (!writeRegister(0x0D + i, value)) {
					return false;
				}
			}

			return true;
		}

		public boolean setGain(int gain) {
			CavanAndroid.eLog("setGain = " + gain);

			int value = (gain & 0x03) << 14;

			for (int i = 0; i < 4; i++) {
				if (!writeRegister(0x11 + i, value)) {
					return false;
				}
			}

			return true;
		}

		public boolean setCapacityDac(int offset) {
			CavanAndroid.eLog("setCapacityDac = " + offset);

			for (int i = 0; i < 4; i++) {
				int value = i << 13 | 4 << 10 | offset;

				if (!writeRegister(0x09 + i, value)) {
					return false;
				}
			}

			return true;
		}

		@Override
		protected int readRegister(int addr) {
			return readValueBe16(addr);
		}

		@Override
		protected boolean writeRegister(int addr, int value) {
			return writeValueBe16(addr, value);
		}
	}
}
