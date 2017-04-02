package com.jwaoo.android;

import java.util.UUID;

import android.bluetooth.BluetoothDevice;
import android.content.Context;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanBleGatt;
import com.cavan.java.CavanArray;
import com.cavan.java.CavanByteCache;
import com.cavan.java.CavanHexFile;
import com.cavan.java.CavanJava;
import com.cavan.java.CavanOverrideQueue;
import com.cavan.java.CavanProgressListener;
import com.cavan.java.CavanString;
import com.cavan.java.VoltageCapacityTable;
import com.cavan.java.VoltageCapacityTable.Entry;

public class JwaooBleToy extends CavanBleGatt {

	private static final int SENSOR_QUEUE_SIZE = 10;
	private static final int SENSOR_QUEUE_MIN = 1;
	private static final int SENSOR_QUEUE_MAX = 1;
	private static final int SENSOR_DELAY_STEP = 100000;

	public static final String[] BT_NAMES = {
		"JwaooToy", "SenseTube", "SenseBand"
	};

	public static final UUID[] BT_UUIDS = {
		UUID.fromString("00001888-0000-1000-8000-00805f9b34fb")
	};

	public static final int MOTO_MODE_MAX = 6;
	public static final int MOTO_LEVEL_MAX = 18;
	public static final int SENSOR_DATA_SKIP = 20;
	public static final long DATA_TIMEOUT = 5000;
	public static final long JWAOO_TOY_TIME_MIN = 1000;
	public static final long JWAOO_TOY_TIME_MAX_VALUE = 3000;
	public static final long JWAOO_TOY_TIME_MAX_FREQ = 5000;
	public static final double JWAOO_TOY_ACCEL_VALUE_FUZZ = 2.0;
	public static final double JWAOO_TOY_DEPTH_VALUE_FUZZ = 6.0;

	public static final int SENSOR_DELAY_DEFAULT = 20;

	public static final int LED_BATT = 1;
	public static final int LED_BT = 2;

	public static final int KEYCODE_UP = 0;
	public static final int KEYCODE_O = 1;
	public static final int KEYCODE_DOWN = 2;
	public static final int KEYCODE_MAX = 3;

	public static final int BATTERY_STATE_NORMAL = 0;
	public static final int BATTERY_STATE_LOW = 1;
	public static final int BATTERY_STATE_CHARGING = 2;
	public static final int BATTERY_STATE_FULL = 3;

	public static final int MOTO_MODE_IDLE = 0;
	public static final int MOTO_MODE_LINE = 1;
	public static final int MOTO_MODE_SAWTOOTH = 2;
	public static final int MOTO_MODE_SAWTOOTH_FAST = 3;
	public static final int MOTO_MODE_SQUARE = 4;
	public static final int MOTO_MODE_SQUARE_FAST = 5;
	public static final int MOTO_MODE_RAND = 6;

	public static final String DEVICE_NAME_COMMON = "JwaooToy";
	public static final String DEVICE_NAME_K100 = "K100";
	public static final String DEVICE_NAME_K101 = "K101";
	public static final String DEVICE_NAME_MODEL06 = "MODEL-06";
	public static final String DEVICE_NAME_MODEL10 = "MODEL-10";

	public static final int DEVICE_ID_K100 = 100;
	public static final int DEVICE_ID_K101 = 101;
	public static final int DEVICE_ID_MODEL06 = 102;
	public static final int DEVICE_ID_MODEL10 = 105;

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
	public static final byte JWAOO_TOY_CMD_SUSPEND_DELAY = 7;
	public static final byte JWAOO_TOY_CMD_APP_DATA = 8;
	public static final byte JWAOO_TOY_CMD_APP_SETTINGS = 9;
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
	public static final byte JWAOO_TOY_CMD_BATT_SHUTDOWN_VOLTAGE = 62;
	public static final byte JWAOO_TOY_CMD_SENSOR_ENABLE = 70;
	public static final byte JWAOO_TOY_CMD_MOTO_SET_MODE = 80;
	public static final byte JWAOO_TOY_CMD_MOTO_GET_MODE = 81;
	public static final byte JWAOO_TOY_CMD_MOTO_EVENT_ENABLE = 82;
	public static final byte JWAOO_TOY_CMD_MOTO_SPEED_TABLE = 83;
	public static final byte JWAOO_TOY_CMD_KEY_CLICK_ENABLE = 90;
	public static final byte JWAOO_TOY_CMD_KEY_LONG_CLICK_ENABLE = 91;
	public static final byte JWAOO_TOY_CMD_KEY_MULTI_CLICK_ENABLE = 92;
	public static final byte JWAOO_TOY_CMD_KEY_LOCK = 93;
	public static final byte JWAOO_TOY_CMD_KEY_REPORT_ENABLE = 94;
	public static final byte JWAOO_TOY_CMD_KEY_SETTINGS = 95;
	public static final byte JWAOO_TOY_CMD_GPIO_GET = 100;
	public static final byte JWAOO_TOY_CMD_GPIO_SET = 101;
	public static final byte JWAOO_TOY_CMD_GPIO_CFG = 102;

	public static final byte JWAOO_TOY_EVT_BATT_INFO = 1;
	public static final byte JWAOO_TOY_EVT_KEY_STATE = 2;
	public static final byte JWAOO_TOY_EVT_KEY_CLICK = 3;
	public static final byte JWAOO_TOY_EVT_KEY_LONG_CLICK = 4;
	public static final byte JWAOO_TOY_EVT_UPGRADE_COMPLETE = 5;
	public static final byte JWAOO_TOY_EVT_MOTO_STATE_CHANGED = 6;

	private static final Entry[] sBatteryCapacityEntriesCR1632 = {
		new Entry(2.0, 0),
		new Entry(2.75, 10),
		new Entry(2.87, 25),
		new Entry(2.9, 85),
		new Entry(3.0, 100),
	};

	public static final JwaooBleToyEventListener sEventListenerDummy = new JwaooBleToyEventListener() {

		@Override
		public void onUpgradeComplete(boolean success) {
			CavanAndroid.dLog("onUpgradeComplete: success = " + success);
		}

		@Override
		public void onSensorDataReceived(JwaooToySensor sensor, byte[] data) {
			// CavanAndroid.dLog("onSensorDataReceived: length = " + data.length);
		}

		@Override
		public void onMotoStateChanged(int mode, int level) {
			CavanAndroid.dLog("onMotoStateChanged: mode = " +  mode + ", level = " + level);
		}

		@Override
		public void onKeyStateChanged(int code, int state) {
			CavanAndroid.dLog("onKeyStateChanged: code = " + code + ", state = " + state);
		}

		@Override
		public void onKeyLongClicked(int code) {
			CavanAndroid.dLog("onKeyLongClicked: code = " + code);
		}

		@Override
		public void onKeyClicked(int code, int count) {
			CavanAndroid.dLog("onKeyClicked: code = " + code + ", count = " + count);
		}

		@Override
		public boolean onInitialize() {
			CavanAndroid.dLog("onInitialize");
			return true;
		}

		@Override
		public void onDebugDataReceived(byte[] data) {
			CavanAndroid.dLog("onDebugDataReceived: length = " + data.length);
		}

		@Override
		public void onConnectionStateChanged(boolean connected) {
			CavanAndroid.dLog("onConnectionStateChanged: connected = " + connected);
		}

		@Override
		public void onConnectFailed() {
			CavanAndroid.dLog("onConnectFailed");
		}

		@Override
		public void onBatteryStateChanged(int state, int level, double voltage) {
			CavanAndroid.dLog("onBatteryStateChanged: state = " + state + ", level = " + level + ", voltage = " + voltage);
		}

		@Override
		public void onBluetoothAdapterStateChanged(boolean enabled) {
			CavanAndroid.dLog("onBluetoothAdapterStateChanged: enabled = " + enabled);
		}
	};

	public interface JwaooBleToyEventListener {
		boolean onInitialize();
		void onConnectFailed();
		void onConnectionStateChanged(boolean connected);
		void onBluetoothAdapterStateChanged(boolean enabled);
		void onBatteryStateChanged(int state, int level, double voltage);
		void onKeyStateChanged(int code, int state);
		void onKeyClicked(int code, int count);
		void onKeyLongClicked(int code);
		void onMotoStateChanged(int mode, int level);
		void onUpgradeComplete(boolean success);
		void onSensorDataReceived(JwaooToySensor sensor, byte[] data);
		void onDebugDataReceived(byte[] data);
	}

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
	protected int mSensorDelayNanos;
	protected boolean mSensorSpeedOptimize;
	protected long mSensorDelayMillis = SENSOR_DELAY_DEFAULT;
	protected VoltageCapacityTable mBatteryCapacityTable = new VoltageCapacityTable(3200, 4200);
	private CavanOverrideQueue<byte[]> mSensorDataQueue = new CavanOverrideQueue<byte[]>(SENSOR_QUEUE_SIZE);
	private SensorSpeedOptimizeThread mSensorOptimizeThread;

	private JwaooBleToyEventListener mEventListener = sEventListenerDummy;

	private CavanBleDataListener mEventDataListener = new CavanBleDataListener() {

		@Override
		public void onDataReceived(byte[] data) {
			onEventReceived(data);
		}
	};

	private CavanBleDataListener mSensorDataListener = new CavanBleDataListener() {

		@Override
		public void onDataReceived(byte[] data) {
			if (mSensorDataSkip > 0) {
				mSensorDataSkip--;
			} else if (mSensor != null) {
				if (mSensorSpeedOptimize) {
					mSensorDataQueue.addOverride(data.clone());
				} else {
					putSensorData(data);
				}
			}
		}
	};

	public void setEventListener(JwaooBleToyEventListener listener) {
		if (listener != null) {
			mEventListener = listener;
		} else {
			mEventListener = sEventListenerDummy;
		}
	}

	@Override
	protected boolean onInitialize() {
		return mEventListener.onInitialize();
	}

	@Override
	protected void onConnectionStateChange(boolean connected) {
		mEventListener.onConnectionStateChanged(connected);
	}

	@Override
	protected void onConnectFailed() {
		mEventListener.onConnectFailed();
	}

	@Override
	protected void onBluetoothAdapterStateChanged(boolean enabled) {
		mEventListener.onBluetoothAdapterStateChanged(enabled);
	}

	protected void onBatteryStateChanged(int state, int level, double voltage) {
		mEventListener.onBatteryStateChanged(state, level, voltage);
	}

	protected void onKeyStateChanged(int code, int state) {
		mEventListener.onKeyStateChanged(code, state);
	}

	protected void onKeyClicked(int code, int count) {
		mEventListener.onKeyClicked(code, count);
	}

	protected void onKeyLongClicked(int code) {
		mEventListener.onKeyLongClicked(code);
	}

	protected void onMotoStateChanged(int mode, int level) {
		mEventListener.onMotoStateChanged(mode, level);
	}

	protected void onUpgradeComplete(boolean success) {
		mEventListener.onUpgradeComplete(success);
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

	private void putSensorData(byte[] data) {
		mSensor.putBytes(data);
		onSensorDataReceived(data);
	}

	protected void onSensorDataReceived(byte[] data) {
		mEventListener.onSensorDataReceived(mSensor, data);
	}

	protected void onDebugDataReceived(byte[] data) {
		mEventListener.onDebugDataReceived(data);
	}

	public JwaooBleToy(Context context, BluetoothDevice device, UUID uuid) {
		super(context, device, uuid);
	}

	public JwaooBleToy(Context context, BluetoothDevice device) {
		this(context, device, UUID_SERVICE);
	}

	public JwaooBleToy(Context context) {
		this(context, null);
	}

	public JwaooBleToy(BluetoothDevice device) {
		this(null, device, UUID_SERVICE);
	}

	public JwaooBleToy() {
		this(null, null);
	}

	@Override
	protected void finalize() throws Throwable {
		setSensorSpeedOptimizeEnable(false);
		super.finalize();
	}

	public JwaooToySensor getSensor() {
		return mSensor;
	}

	public void setSensor(JwaooToySensor sensor) {
		if (sensor != null) {
			mSensor = sensor;
		}
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

		CavanAndroid.dLog("Firmware size = " + bytes.length);

		CavanAndroid.dLog("Flash id = " + Integer.toHexString(getFlashId()));
		CavanAndroid.dLog("Flash size = " + getFlashSize());
		CavanAndroid.dLog("Flash page size = " + getFlashPageSize());

		CavanAndroid.dLog("setFlashWriteEnable");

		listener.addProgress();

		if (!setFlashWriteEnable(true)) {
			CavanAndroid.eLog("Failed to setFlashWriteEnable true");
			return false;
		}

		listener.addProgress();

		CavanAndroid.dLog("startFlashWrite");

		if (!startFlashWrite()) {
			CavanAndroid.eLog("Failed to startFlashWrite");
			return false;
		}

		listener.addProgress();

		CavanAndroid.dLog("eraseFlash");

		if (!eraseFlash()) {
			CavanAndroid.eLog("Failed to eraseFlash");
			return false;
		}

		listener.addProgress();

		mFlashCrc = (byte) 0xFF;

		CavanAndroid.dLog("writeFlashHeader");

		if (!writeFlashHeader(bytes.length)) {
			CavanAndroid.eLog("Failed to writeFlashHeader");
			return false;
		}

		listener.addProgress();

		CavanAndroid.dLog("writeFlash body");

		if (!writeFlash(bytes, listener)) {
			CavanAndroid.eLog("Failed to writeFlash body");
			return false;
		}

		CavanAndroid.dLog("finishWriteFlash");

		if (!finishWriteFlash(bytes.length + 8)) {
			CavanAndroid.eLog("Failed to finishWriteFlash");
			return false;
		}

		for (int i = 0; i < 10; i++) {
			CavanAndroid.dLog("wait upgrade complete" + i);

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

	public void setSensorSpeedOptimizeEnable(boolean enable) {
		mSensorSpeedOptimize = enable;

		if (enable) {
			if (mSensorOptimizeThread == null) {
				mSensorOptimizeThread = new SensorSpeedOptimizeThread();
				mSensorOptimizeThread.start();
			}
		} else if (mSensorOptimizeThread != null) {
			mSensorDataQueue.addOverride(null);
		}
	}

	public boolean setSensorEnable(boolean enable, int delay) {
		mSensorDataSkip = SENSOR_DATA_SKIP;
		mSensorDelayMillis = delay;
		mSensorDelayNanos = 0;

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

	public JwaooToyKeySettings readKeySettings() {
		JwaooToyResponse response = mCommand.send(JWAOO_TOY_CMD_KEY_SETTINGS);
		if (response == null) {
			return null;
		}

		return response.getKeySettings();
	}

	public boolean writeKeySettings(JwaooToyKeySettings settings) {
		return mCommand.readBool(settings.buildCommand());
	}

	public boolean readSpeedTable(int index, short[] table, int offset) {
		return mCommand.readArray16(new byte[] { JWAOO_TOY_CMD_MOTO_SPEED_TABLE, (byte) index }, table, offset, 9);
	}

	public short[] readSpeedTable() {
		short[] table = new short[18];

		if (readSpeedTable(1, table, 0) && readSpeedTable(10, table, 9)) {
			return table;
		}

		return null;
	}

	public boolean writeSpeedTable(int index, short[] array, int offset) {
		CavanByteCache cache = new CavanByteCache(20);
		cache.writeValue8(JWAOO_TOY_CMD_MOTO_SPEED_TABLE);
		cache.writeValue8((byte) index);

		for (int end = offset + 9; offset < end; offset++) {
			cache.writeValue16(array[offset]);
		}

		return mCommand.readBool(cache.getBytes());
	}

	public boolean writeSpeedTable(short[] table) {
		return writeSpeedTable(1, table, 0) && writeSpeedTable(10, table, 9);
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

		JwaooToyTestResult result = response.getTestResult();
		if (result != null) {
			return result;
		}

		return new JwaooToyTestResult();
	}

	public boolean writeTestResult(JwaooToyTestResult result) {
		mCommand.readBool(result.buildCommand());
		return true;
	}

	public JwaooToyAppSettings readAppSettings() {
		JwaooToyResponse response = mCommand.send(JWAOO_TOY_CMD_APP_SETTINGS);
		if (response == null) {
			return null;
		}

		return response.getAppSettings();
	}

	public boolean writeAppSettings(JwaooToyAppSettings settings) {
		return mCommand.readBool(settings.buildCommand());
	}

	public int getDeviveId() {
		return mDeviceId;
	}

	public String getDeviceName() {
		return mDeviceName;
	}

	public int getDepthSteps() {
		return mSensor.getCapacityValueCount();
	}

	public boolean getKeyLockState() {
		return mCommand.readValue8(JWAOO_TOY_CMD_KEY_LOCK, (byte) 0) > 0;
	}

	public boolean setKeyLock(boolean enable) {
		return mCommand.readBool(JWAOO_TOY_CMD_KEY_LOCK, enable);
	}

	public boolean setKeyReportEnable(int mask) {
		return mCommand.readBool(JWAOO_TOY_CMD_KEY_REPORT_ENABLE, (byte) mask);
	}

	public boolean writeAppData(byte[] bytes) {
		byte[] command = new byte[bytes.length + 1];

		command[0] = JWAOO_TOY_CMD_APP_DATA;
		CavanArray.copy(bytes, 0, command, 1, bytes.length);

		return mCommand.readBool(command);
	}

	public byte[] readAppData() {
		return mCommand.readData(JWAOO_TOY_CMD_APP_DATA);
	}

	public boolean setSuspendDelay(int delay) {
		return mCommand.readBool(JWAOO_TOY_CMD_SUSPEND_DELAY, (short) delay);
	}

	public int getSuspendDelay() {
		return mCommand.readValue16(JWAOO_TOY_CMD_SUSPEND_DELAY, (short) -1);
	}

	public boolean setShutdownVoltage(int voltage) {
		return mCommand.readBool(JWAOO_TOY_CMD_BATT_SHUTDOWN_VOLTAGE, (short) voltage);
	}

	public int getShutdownVoltage() {
		return mCommand.readValue16(JWAOO_TOY_CMD_BATT_SHUTDOWN_VOLTAGE, (short) -1);
	}

	public static String getBatteryStateString(int state) {
		switch (state) {
		case BATTERY_STATE_NORMAL:
			return "Discharging";

		case BATTERY_STATE_LOW:
			return "Low";

		case BATTERY_STATE_CHARGING:
			return "Charging";

		case BATTERY_STATE_FULL:
			return "Full";

		default:
			return "Unknown";
		}
	}

	public double getBatteryCapacityByVoltage(double voltage) {
		return mBatteryCapacityTable.getCapacity(voltage);
	}

	public JwaooToyBatteryInfo getBatteryInfo() {
		JwaooToyResponse response = mCommand.send(JWAOO_TOY_CMD_BATT_INFO);
		if (response == null) {
			return null;
		}

		return response.getBatteryInfo();
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

		if (!mCharEvent.setDataListener(mEventDataListener)) {
			CavanAndroid.eLog("Failed to mCharEvent.setDataListener");
			return false;
		}

		if (!mCharSensor.setDataListener(mSensorDataListener)) {
			CavanAndroid.eLog("Failed to mCharSensor.setDataListener");
			return false;
		}

		setReady(true);

		String identify = doIdentify();
		if (identify == null) {
			CavanAndroid.eLog("Failed to doIdentify");
			return false;
		}

		CavanAndroid.dLog("identify = " + identify);

		if (identify.equals(DEVICE_NAME_COMMON) || identify.equals(DEVICE_NAME_K100)) {
			mSensor = new JwaooToySensorK100();
			mDeviceName = DEVICE_NAME_K100;
			mDeviceId = DEVICE_ID_K100;
		} else if (identify.equals(DEVICE_NAME_K101)) {
			mSensor = new JwaooToySensorK101();
			mDeviceName = DEVICE_NAME_K101;
			mDeviceId = DEVICE_ID_K101;
		} else if (identify.equals(DEVICE_NAME_MODEL06)) {
			mSensor = new JwaooToySensorModel06();
			mDeviceName = DEVICE_NAME_MODEL06;
			mDeviceId = DEVICE_ID_MODEL06;
		} else if (identify.equals(DEVICE_NAME_MODEL10)) {
			mSensor = new JwaooToySensorModel10();
			mDeviceName = DEVICE_NAME_MODEL10;
			mDeviceId = DEVICE_ID_MODEL10;
			mBatteryCapacityTable = new VoltageCapacityTable(sBatteryCapacityEntriesCR1632);
		} else {
			CavanAndroid.eLog("Invalid identify");
			return false;
		}

		return true;
	}

	// ================================================================================

	public static class JwaooToyBatteryInfo {

		private int mState;
		private int mCapacity;
		private double mVoltage;

		public static JwaooToyBatteryInfo getInstance(byte[] bytes, int start) {
			return new JwaooToyBatteryInfo(bytes, start);
		}

		private JwaooToyBatteryInfo(byte[] bytes, int start) {
			CavanByteCache cache = new CavanByteCache(bytes);
			cache.setOffset(start);

			mState = cache.readValue8();
			mCapacity = cache.readValue8();
			mVoltage = ((double) cache.readValue16()) / 1000;
		}

		public int getState() {
			return mState;
		}

		public int getCapacity() {
			return mCapacity;
		}

		public double getVoltage() {
			return mVoltage;
		}

		@Override
		public String toString() {
			StringBuilder builder = new StringBuilder();

			builder.append('[');
			builder.append("state:").append(getBatteryStateString(mState)).append(", ");
			builder.append("capacity:").append(mCapacity).append(", ");
			builder.append("voltage:").append(mVoltage);
			builder.append(']');

			return builder.toString();
		}
	}

	public static class JwaooToyAppSettings {

		public static final int COMMAND_LENGTH = 11;

		private int mSuspendDelay;
		private int mShutdownVoltage;
		private int mBtLedCloseTime;
		private int mBtLedOpenTime;
		private int mMotoRandMax;
		private int mMotoRandDelay;
		private int mMotoSpeedMin;

		private static JwaooToyAppSettings getInstance(byte[] response) {
			return new JwaooToyAppSettings(response);
		}

		private JwaooToyAppSettings(byte[] response) {
			CavanByteCache cache = new CavanByteCache(response);
			cache.setOffset(2);

			mSuspendDelay = cache.readValue16();
			mShutdownVoltage = cache.readValue16();
			mBtLedCloseTime = cache.readValue16();
			mBtLedOpenTime = cache.readValue8();
			mMotoRandDelay = cache.readValue8();
			mMotoRandMax = cache.readValue8();
			mMotoSpeedMin = cache.readValue8();
		}

		public byte[] buildCommand() {
			CavanByteCache cache = new CavanByteCache(COMMAND_LENGTH);
			cache.writeValue8(JWAOO_TOY_CMD_APP_SETTINGS);
			cache.writeValue16((short) mSuspendDelay);
			cache.writeValue16((short) mShutdownVoltage);
			cache.writeValue16((short) mBtLedCloseTime);
			cache.writeValue8((byte) mBtLedOpenTime);
			cache.writeValue8((byte) mMotoRandDelay);
			cache.writeValue8((byte) mMotoRandMax);
			cache.writeValue8((byte) mMotoSpeedMin);
			return cache.getBytes();
		}

		public int getSuspendDelay() {
			return mSuspendDelay;
		}

		public void setSuspendDelay(int delay) {
			mSuspendDelay = delay;
		}

		public int getShutdownVoltage() {
			return mShutdownVoltage;
		}

		public void setShutdownVoltage(int voltage) {
			mShutdownVoltage = voltage;
		}

		public int getBtLedOpenTime() {
			return mBtLedOpenTime;
		}

		public void setBtLedOpenTime(int time) {
			mBtLedOpenTime = time;
		}

		public int getBtLedCloseTime() {
			return mBtLedCloseTime;
		}

		public void setBtLedCloseTime(int time) {
			mBtLedCloseTime = time;
		}

		public boolean commit(JwaooBleToy ble) {
			return ble.writeAppSettings(this);
		}

		@Override
		public String toString() {
			StringBuilder builder = new StringBuilder();

			builder.append("suspend_delay: ");
			builder.append(mSuspendDelay);
			builder.append(", shutdown_voltage: ");
			builder.append(mShutdownVoltage);
			builder.append(", bt_led_open_time: ");
			builder.append(mBtLedOpenTime);
			builder.append(", bt_led_close_time: ");
			builder.append(mBtLedCloseTime);
			builder.append(", moto_rand_delay: ");
			builder.append(mMotoRandDelay);
			builder.append(", moto_rand_max: ");
			builder.append(mMotoRandMax);
			builder.append(", moto_speed_min: ");
			builder.append(mMotoSpeedMin);

			return builder.toString();
		}
	}

	public static class JwaooToyKeySettings {

		public static final int COMMAND_LENGTH = 10;

		private boolean mClickEnable;
		private boolean mMultiClickEnable;
		private int mMultiClickDelay;
		private boolean mLongClickEnable;
		private int mLongClickDelay;
		private int mLedBlinkDelay;

		private static JwaooToyKeySettings getInstance(byte[] response) {
			return new JwaooToyKeySettings(response);
		}

		private JwaooToyKeySettings(byte[] response) {
			CavanByteCache cache = new CavanByteCache(response);
			cache.setOffset(2);

			mClickEnable = cache.readBool();
			mMultiClickEnable = cache.readBool();
			mMultiClickDelay = cache.readValue16();
			mLongClickEnable = cache.readBool();
			mLongClickDelay = cache.readValue16();
			mLedBlinkDelay = cache.readValue16();
		}

		public byte[] buildCommand() {
			CavanByteCache cache = new CavanByteCache(COMMAND_LENGTH);
			cache.writeValue8(JWAOO_TOY_CMD_KEY_SETTINGS);
			cache.writeBool(mClickEnable);
			cache.writeBool(mMultiClickEnable);
			cache.writeValue16((short) mMultiClickDelay);
			cache.writeBool(mLongClickEnable);
			cache.writeValue16((short) mLongClickDelay);
			cache.writeValue16((short) mLedBlinkDelay);
			return cache.getBytes();
		}

		public boolean isClickEnabled() {
			return mClickEnable;
		}

		public void setClickEnable(boolean enable) {
			mClickEnable = enable;
		}

		public void setClickEnable() {
			mClickEnable = true;
		}

		public boolean isMultiClickEnabled() {
			return mMultiClickEnable;
		}

		public void setMultiClickEnable(boolean enable) {
			mMultiClickEnable = enable;
		}

		public void setMultiClickEnable() {
			mMultiClickEnable = true;
		}

		public int getMultiClickDelay() {
			return mMultiClickDelay;
		}

		public void setMultiClickDelay(int delay) {
			mMultiClickDelay = delay;
		}

		public boolean isLongClickEnabled() {
			return mLongClickEnable;
		}

		public void setLongClickEnable(boolean enable) {
			mLongClickEnable = enable;
		}

		public void setLongClickEnable() {
			mLongClickEnable = true;
		}

		public int getLongClickDelay() {
			return mLongClickDelay;
		}

		public void setLongClickDelay(int delay) {
			mLongClickDelay = delay;
		}

		public boolean commit(JwaooBleToy ble) {
			return ble.writeKeySettings(this);
		}

		public void setLedBlinkDelay(int delay) {
			mLedBlinkDelay = delay;
		}

		public int getLedBlinkDelay() {
			return mLedBlinkDelay;
		}

		@Override
		public String toString() {
			StringBuilder builder = new StringBuilder();

			builder.append("click_enable: ");
			builder.append(mClickEnable);
			builder.append(", multi_click_enable: ");
			builder.append(mMultiClickEnable);
			builder.append(", multi_click_delay: ");
			builder.append(mMultiClickDelay);
			builder.append(", long_click_enable: ");
			builder.append(mLongClickEnable);
			builder.append(", long_click_delay: ");
			builder.append(mLongClickDelay);
			builder.append(", blink_delay: ");
			builder.append(mLedBlinkDelay);

			return builder.toString();
		}
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

		public JwaooToyTestResult() {
			mTestValid = 0;
			mTestResult = 0;
		}

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

		public boolean getArray16(short[] array, int offset, int count) {
			if (getType() != JWAOO_TOY_RSP_DATA || length() != (count * 2 + 2)) {
				return false;
			}

			for (int i = 2; i < mBytes.length; i += 2, offset++) {
				array[offset] = CavanJava.buildValue16(mBytes, i);
			}

			return true;
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

			return CavanArray.copySkip(mBytes, 2);
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

		public JwaooToyAppSettings getAppSettings() {
			if (getType() != JWAOO_TOY_RSP_DATA) {
				return null;
			}

			return JwaooToyAppSettings.getInstance(mBytes);
		}

		public JwaooToyKeySettings getKeySettings() {
			if (getType() != JWAOO_TOY_RSP_DATA) {
				return null;
			}

			return JwaooToyKeySettings.getInstance(mBytes);
		}

		public JwaooToyBatteryInfo getBatteryInfo() {
			if (getType() != JWAOO_TOY_RSP_DATA) {
				return null;
			}

			return JwaooToyBatteryInfo.getInstance(mBytes, 2);
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

		public static boolean getArray16(JwaooToyResponse response, short[] array, int offset, int count) {
			if (response == null) {
				return false;
			}

			return response.getArray16(array, offset, count);
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
					CavanAndroid.dLog("response: command = " + response[0] + ", type = " + response[1] + ", length = " + response.length);
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
			CavanArray.copy(data, 0, command, 1, data.length);

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

		public boolean readArray16(byte[] command, short[] array, int offset, int count) {
			return JwaooToyResponse.getArray16(send(command), array, offset, count);
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
			CavanArray.copy(data, 0, command, 3, data.length);

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

			CavanAndroid.dLog(String.format("updateBits8: addr = 0x%02x, value: 0x%02x => 0x%02x", addr, valueOld, value));

			if (value == valueOld) {
				return true;
			}

			return writeValue8(addr, value);
		}

		public boolean updateBits16(int addr, int value, int mask) {
			int valueOld = readValue16(addr);

			value |= (valueOld & (~mask));

			CavanAndroid.dLog(String.format("updateBits16: addr = 0x%02x, value: 0x%04x => 0x%04x", addr, valueOld, value));

			if (value == valueOld) {
				return true;
			}

			return writeValue16(addr, value);
		}

		public boolean updateBits32(int addr, int value, int mask) {
			int valueOld = readValue32(addr);

			value |= (valueOld & (~mask));

			CavanAndroid.dLog(String.format("updateBits32: addr = 0x%02x, value: 0x%08x => 0x%08x", addr, valueOld, value));

			if (value == valueOld) {
				return true;
			}

			return writeValue32(addr, value);
		}

		public boolean updateBitsBe16(int addr, int value, int mask) {
			int valueOld = readValueBe16(addr);

			value |= (valueOld & (~mask));

			CavanAndroid.dLog(String.format("updateBitsBe16: addr = 0x%02x, value: 0x%04x => 0x%04x", addr, valueOld, value));

			if (value == valueOld) {
				return true;
			}

			return writeValueBe16(addr, value);
		}

		public boolean updateBitsBe32(int addr, int value, int mask) {
			int valueOld = readValueBe32(addr);

			value |= (valueOld & (~mask));

			CavanAndroid.dLog(String.format("updateBits32: addr = 0x%02x, value: 0x%08x => 0x%08x", addr, valueOld, value));

			if (value == valueOld) {
				return true;
			}

			return writeValueBe32(addr, value);
		}

		public boolean updateRegister(int addr, int value, int mask) {
			int valueOld = readRegister(addr);

			value |= (valueOld & (~mask));

			CavanAndroid.dLog(String.format("updateRegister: addr = 0x%02x, value: 0x%08x => 0x%08x", addr, valueOld, value));

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

			CavanAndroid.dLog(String.format("MPU6050: Chip ID = 0x%02x", id));

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

			CavanAndroid.dLog(String.format("BMI160: Chip ID = 0x%02x", id));

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
			CavanAndroid.dLog(String.format("FDC1004: Device ID = 0x%04x", id));

			if (id != 0x1004) {
				CavanAndroid.eLog("Invalid device id");
				return false;
			}

			id = readManufacturerId();
			CavanAndroid.dLog(String.format("FDC1004: Manufacturer ID = 0x%04x", id));

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
			CavanAndroid.dLog("setOffset = " + offset);

			int value = (offset & 0x1F) << 11;

			for (int i = 0; i < 4; i++) {
				if (!writeRegister(0x0D + i, value)) {
					return false;
				}
			}

			return true;
		}

		public boolean setGain(int gain) {
			CavanAndroid.dLog("setGain = " + gain);

			int value = (gain & 0x03) << 14;

			for (int i = 0; i < 4; i++) {
				if (!writeRegister(0x11 + i, value)) {
					return false;
				}
			}

			return true;
		}

		public boolean setCapacityDac(int offset) {
			CavanAndroid.dLog("setCapacityDac = " + offset);

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

	public class SensorSpeedOptimizeThread extends Thread {

		public SensorSpeedOptimizeThread() {
			super();
			setPriority(MAX_PRIORITY);
		}

		@Override
		public void run() {
			mSensorDataQueue.clear();

			while (true) {
				byte[] bytes = mSensorDataQueue.removeBlocked();
				if (bytes == null) {
					break;
				}

				putSensorData(bytes);

				int size = mSensorDataQueue.size();
				if (size > SENSOR_QUEUE_MAX) {
					mSensorDelayNanos -= SENSOR_DELAY_STEP;

					if (mSensorDelayNanos < 0) {
						if (mSensorDelayMillis > 0) {
							mSensorDelayMillis--;
							mSensorDelayNanos += 1000000;
						} else {
							mSensorDelayMillis = 0;
							mSensorDelayNanos = 0;
						}
					}

					if (size > 3) {
						CavanAndroid.dLog("size = " + size + ", millis = " + mSensorDelayMillis + ", nanos = " + mSensorDelayNanos);
					}
				} else if (size < SENSOR_QUEUE_MIN) {
					mSensorDelayNanos += SENSOR_DELAY_STEP;

					if (mSensorDelayNanos > 999999) {
						mSensorDelayNanos -= 1000000;
						mSensorDelayMillis++;
					}
				}

				try {
					Thread.sleep(mSensorDelayMillis, mSensorDelayNanos);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}

			mSensorOptimizeThread = null;
		}
	};
}
