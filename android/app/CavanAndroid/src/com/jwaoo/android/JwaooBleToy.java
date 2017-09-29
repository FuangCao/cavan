package com.jwaoo.android;

import java.util.Date;
import java.util.UUID;

import android.content.Context;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanBleChar;
import com.cavan.android.CavanBleChar.CavanBleDataListener;
import com.cavan.android.CavanBleGatt;
import com.cavan.android.CavanSingleInstanceHelper;
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
		"JwaooToy", "SenseTube", "SenseBand", "Sensevibe warm"
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
	public static final int MOTO_MODE_LINE_MAIN = 7;
	public static final int MOTO_MODE_LINE_AUX = 8;
	public static final int MOTO_MODE_USER = 9;

	public static final String DEVICE_NAME_COMMON = "JwaooToy";
	public static final String DEVICE_NAME_K100 = "K100";
	public static final String DEVICE_NAME_K101 = "K101";
	public static final String DEVICE_NAME_MODEL06 = "MODEL-06";
	public static final String DEVICE_NAME_MODEL10 = "MODEL-10";
	public static final String DEVICE_NAME_MODEL11 = "MODEL-11";
	public static final String DEVICE_NAME_MODEL01 = "MODEL-01";
	public static final String DEVICE_NAME_MODEL03 = "MODEL-03";

	public static final int DEVICE_ID_K100 = 100;
	public static final int DEVICE_ID_K101 = 101;
	public static final int DEVICE_ID_MODEL06 = 102;
	public static final int DEVICE_ID_MODEL10 = 105;
	public static final int DEVICE_ID_MODEL11 = 106;
	public static final int DEVICE_ID_MODEL01 = 107;
	public static final int DEVICE_ID_MODEL03 = 108;

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
	public static final byte JWAOO_TOY_CMD_HEATER_ENABLE = 71;
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
		public void onBatteryStateChanged(JwaooToyBatteryInfo info) {
			CavanAndroid.dLog("onBatteryStateChanged: " + info);
		}

		@Override
		public void onConnectionStateChanged(boolean connected) {}

		@Override
		public void onConnectFailed() {}

		@Override
		public void onBluetoothAdapterStateChanged(boolean enabled) {}
	};

	private static CavanSingleInstanceHelper<JwaooBleToy> mInstanceHelper = new CavanSingleInstanceHelper<JwaooBleToy>() {

		@Override
		public JwaooBleToy createInstance(Object... args) {
			return new JwaooBleToy((Context) args[0]);
		}
	};

	public static JwaooBleToy getInstance(Context context) {
		return mInstanceHelper.getInstance(context);
	}

	public interface JwaooBleToyEventListener extends CavanBleGattEventListener {
		void onBatteryStateChanged(JwaooToyBatteryInfo info);
		void onKeyStateChanged(int code, int state);
		void onKeyClicked(int code, int count);
		void onKeyLongClicked(int code);
		void onMotoStateChanged(int mode, int level);
		void onUpgradeComplete(boolean success);
		void onSensorDataReceived(JwaooToySensor sensor, byte[] data);
		void onDebugDataReceived(byte[] data);
	}

	private byte mFlashCrc;
	private int mVersion;
	private String mBuildDate;
	private int mDeviceId;
	private String mDeviceName;
	private double mVoltageRatio;
	private int mStateLed;

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
	protected VoltageCapacityTable mBatteryCapacityTable = new VoltageCapacityTable(3.4, 4.15);
	private CavanOverrideQueue<byte[]> mSensorDataQueue = new CavanOverrideQueue<byte[]>(SENSOR_QUEUE_SIZE);
	private SensorSpeedOptimizeThread mSensorOptimizeThread;

	private JwaooBleToyEventListener mEventListener = sEventListenerDummy;

	private CavanBleDataListener mEventDataListener = new CavanBleDataListener() {

		private final byte[] sResponseData = new byte[0];

		@Override
		public void onDataReceived(byte[] data) {
			if (mCharEvent.canWriteNoResponse()) {
				try {
					mCharEvent.writeData(sResponseData, false);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}

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

	protected JwaooBleToy(Context context) {
		super(context, null, UUID_SERVICE);
	}

	@Override
	protected void finalize() throws Throwable {
		setSensorSpeedOptimizeEnable(false);
		super.finalize();
	}

	public void setEventListener(JwaooBleToyEventListener listener) {
		if (listener != null) {
			mEventListener = listener;
		} else {
			mEventListener = sEventListenerDummy;
		}
	}

	public int getStateLed() {
		return mStateLed;
	}

	public void setStateLed(int index) {
		mStateLed = index;
	}

	@Override
	protected boolean onInitialize() throws Exception {
		return mEventListener.onInitialize();
	}

	@Override
	protected void onConnectionStateChanged(boolean connected) {
		super.onConnectionStateChanged(connected);
		mEventListener.onConnectionStateChanged(connected);
	}

	@Override
	protected void onConnectFailed() {
		super.onConnectFailed();
		mEventListener.onConnectFailed();
	}

	@Override
	protected void onBluetoothAdapterStateChanged(boolean enabled) {
		super.onBluetoothAdapterStateChanged(enabled);
		mEventListener.onBluetoothAdapterStateChanged(enabled);
	}

	protected void onBatteryStateChanged(JwaooToyBatteryInfo info) {
		mEventListener.onBatteryStateChanged(info);
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
				if (event.length >= 5) {
					onBatteryStateChanged(new JwaooToyBatteryInfo(event, 1));
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

	public JwaooToySensor getSensor() {
		return mSensor;
	}

	public void setSensor(JwaooToySensor sensor) {
		if (sensor != null) {
			mSensor = sensor;
		}
	}

	public boolean isDeviceModel06() {
		return getDeviveId() == DEVICE_ID_MODEL06;
	}

	public boolean isDeviceModel10() {
		return getDeviveId() == DEVICE_ID_MODEL10;
	}

	public boolean isDeviceSenseTube() {
		return isDeviceModel06();
	}

	public boolean isDeviceSenseBand() {
		return isDeviceModel10();
	}

	public boolean getBool(JwaooToyResponse response) {
		return response != null && response.getBool();
	}

	public byte getValue8(JwaooToyResponse response, byte defValue) {
		if (response == null) {
			return defValue;
		}

		return response.getValue8(defValue);
	}

	public short getValueU8(JwaooToyResponse response, short defValue) {
		if (response == null) {
			return defValue;
		}

		return response.getValueU8(defValue);
	}

	public short getValue16(JwaooToyResponse response, short defValue) {
		if (response == null) {
			return defValue;
		}

		return response.getValue16(defValue);
	}

	public int getValueU16(JwaooToyResponse response, int defValue) {
		if (response == null) {
			return defValue;
		}

		return response.getValueU16(defValue);
	}

	public int getValue32(JwaooToyResponse response, int defValue) {
		if (response == null) {
			return defValue;
		}

		return response.getValue32(defValue);
	}

	public long getValueU32(JwaooToyResponse response, long defValue) {
		if (response == null) {
			return defValue;
		}

		return response.getValueU32(defValue);
	}

	public String getText(JwaooToyResponse response) {
		if (response == null) {
			return null;
		}

		return response.getText();
	}

	public byte[] getData(JwaooToyResponse response) {
		if (response == null) {
			return null;
		}

		return response.getData();
	}

	public boolean getArray16(JwaooToyResponse response, short[] array, int offset, int count) {
		if (response == null) {
			return false;
		}

		return response.getArray16(array, offset, count);
	}

	public String doIdentify() throws Exception {
		return mCommand.readText(JWAOO_TOY_CMD_IDENTIFY);
	}

	public String readBuildDate() throws Exception {
		return mCommand.readText(JWAOO_TOY_CMD_BUILD_DATE);
	}

	public int readVersion() throws Exception {
		return mCommand.readValue32(JWAOO_TOY_CMD_VERSION, 0);
	}

	synchronized public byte[] readFlash(int address) throws Exception {
		if (mCharFlash == null) {
			return null;
		}

		if (!mCommand.readBool(JWAOO_TOY_CMD_FLASH_READ, address)) {
			return null;
		}

		return mCharFlash.readData(DATA_TIMEOUT);
	}

	public int getFlashId() throws Exception {
		return mCommand.readValue32(JWAOO_TOY_CMD_FLASH_ID, -1);
	}

	public int getFlashSize() throws Exception {
		return mCommand.readValue32(JWAOO_TOY_CMD_FLASH_SIZE, -1);
	}

	public int getFlashPageSize() throws Exception {
		return mCommand.readValue32(JWAOO_TOY_CMD_FLASH_PAGE_SIZE, -1);
	}

	public boolean setFlashWriteEnable(boolean enable) throws Exception {
		return mCommand.readBool(JWAOO_TOY_CMD_FLASH_WRITE_ENABLE, enable);
	}

	public boolean eraseFlash() throws Exception {
		return mCommand.readBool(JWAOO_TOY_CMD_FLASH_ERASE);
	}

	public boolean seekFlash(int address) throws Exception {
		return mCommand.readBool(JWAOO_TOY_CMD_FLASH_SEEK, address);
	}

	public boolean startFlashWrite() throws Exception {
		return mCommand.readBool(JWAOO_TOY_CMD_FLASH_WRITE_START);
	}

	synchronized public boolean finishWriteFlash(int length) throws Exception {
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

	synchronized public boolean writeFlash(byte[] data, CavanProgressListener listener) throws Exception {
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

	synchronized private boolean writeFlashHeader(int length) throws Exception {
		length = (length + 7) & (~0x07);

		byte[] header = { 0x70, 0x50, 0x00, 0x00, 0x00, 0x00, (byte) ((length >> 8) & 0xFF), (byte) (length & 0xFF) };

		return writeFlash(header, null);
	}

	synchronized public boolean doOtaUpgrade(String pathname, CavanProgressListener listener) throws Exception {
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

	public boolean setSensorEnable(boolean enable) throws Exception {
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

	public boolean setSensorEnable(boolean enable, int delay) throws Exception {
		mSensorDataSkip = SENSOR_DATA_SKIP;
		mSensorDelayMillis = delay;
		mSensorDelayNanos = 0;

		return mCommand.readBool(JWAOO_TOY_CMD_SENSOR_ENABLE, enable, delay);
	}

	public boolean setMotoMode(int mode, int level) throws Exception {
		byte[] command = { JWAOO_TOY_CMD_MOTO_SET_MODE, (byte) mode, (byte) level };
		return mCommand.readBool(command);
	}

	public boolean setMotoMode(int min, int max, int step, int add_delay, int sub_delay) throws Exception {
		byte[] command = { JWAOO_TOY_CMD_MOTO_SET_MODE, MOTO_MODE_USER, (byte) max, (byte) step, (byte) min, (byte) add_delay, (byte) sub_delay};
		return mCommand.readBool(command);
	}

	public JwaooToyMotoMode getMotoMode() throws Exception {
		JwaooToyResponse response = mCommand.send(JWAOO_TOY_CMD_MOTO_GET_MODE);
		if (response == null) {
			return null;
		}

		return response.getMotoMode();
	}

	public boolean setMotoEventEnable(boolean enable) throws Exception {
		return mCommand.readBool(JWAOO_TOY_CMD_MOTO_EVENT_ENABLE, enable);
	}

	public boolean setFactoryModeEnable(boolean enable) throws Exception {
		return mCommand.readBool(JWAOO_TOY_CMD_FACTORY_ENABLE, enable);
	}

	public boolean setBatteryEventEnable(boolean enable) throws Exception {
		return mCommand.readBool(JWAOO_TOY_CMD_BATT_EVENT_ENABLE, enable);
	}

	public boolean doReboot() throws Exception {
		return mCommand.readBool(JWAOO_TOY_CMD_REBOOT);
	}

	public boolean doShutdown() throws Exception {
		return mCommand.readBool(JWAOO_TOY_CMD_SHUTDOWN);
	}

	public byte[] readBdAddress() throws Exception {
		byte[] bytes = mCommand.readData(JWAOO_TOY_CMD_FLASH_READ_BD_ADDR);
		if (bytes != null && bytes.length == 6) {
			return bytes;
		}

		return null;
	}

	public String readBdAddressString() throws Exception {
		byte[] bytes = readBdAddress();
		if (bytes == null) {
			return null;
		}

		return CavanString.fromBdAddr(bytes);
	}

	public boolean writeBdAddress(byte[] bytes) throws Exception {
		if (!setFlashWriteEnable(true)) {
			CavanAndroid.eLog("Failed to setFlashWriteEnable true");
			return false;
		}

		if (!mCommand.readBool(JWAOO_TOY_CMD_FLASH_WRITE_BD_ADDR, bytes)) {
			return false;
		}

		return setFlashWriteEnable(false);
	}

	public boolean writeBdAddress(String addr) throws Exception {
		byte[] bytes = CavanString.parseBdAddr(addr);
		if (bytes == null) {
			return false;
		}

		return writeBdAddress(bytes);
	}

	public boolean setClickEnable(boolean enable) throws Exception {
		return mCommand.readBool(JWAOO_TOY_CMD_KEY_CLICK_ENABLE, enable);
	}

	public boolean setMultiClickEnable(boolean enable) throws Exception {
		return mCommand.readBool(JWAOO_TOY_CMD_KEY_MULTI_CLICK_ENABLE, enable);
	}

	public boolean setMultiClickEnable(boolean enable, short delay) throws Exception {
		return mCommand.readBool(JWAOO_TOY_CMD_KEY_MULTI_CLICK_ENABLE, enable, delay);
	}

	public boolean setLongClickEnable(boolean enable) throws Exception {
		return mCommand.readBool(JWAOO_TOY_CMD_KEY_LONG_CLICK_ENABLE, enable);
	}

	public boolean setLongClickEnable(boolean enable, short delay) throws Exception {
		return mCommand.readBool(JWAOO_TOY_CMD_KEY_LONG_CLICK_ENABLE, enable, delay);
	}

	public JwaooToyKeySettings readKeySettings() throws Exception {
		JwaooToyResponse response = mCommand.send(JWAOO_TOY_CMD_KEY_SETTINGS);
		if (response == null) {
			return null;
		}

		return response.getKeySettings();
	}

	public boolean readSpeedTable(int index, short[] table, int offset) throws Exception {
		return mCommand.readArray16(new byte[] { JWAOO_TOY_CMD_MOTO_SPEED_TABLE, (byte) index }, table, offset, 9);
	}

	public short[] readSpeedTable() throws Exception {
		short[] table = new short[18];

		if (readSpeedTable(1, table, 0) && readSpeedTable(10, table, 9)) {
			return table;
		}

		return null;
	}

	public boolean writeSpeedTable(int index, short[] array, int offset) throws Exception {
		CavanByteCache cache = new CavanByteCache(20);
		cache.writeValue8(JWAOO_TOY_CMD_MOTO_SPEED_TABLE);
		cache.writeValue8((byte) index);

		for (int end = offset + 9; offset < end; offset++) {
			cache.writeValue16(array[offset]);
		}

		return mCommand.readBool(cache.getBytes());
	}

	public boolean writeSpeedTable(short[] table) throws Exception {
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

	public boolean getGpioValue(int port, int pin) throws Exception {
		byte[] command = { JWAOO_TOY_CMD_GPIO_GET, (byte) port, (byte) pin };
		return mCommand.readValue8(command, (byte) -1) > 0;
	}

	public boolean setGpioValue(int port, int pin, boolean value) throws Exception {
		byte[] command = { JWAOO_TOY_CMD_GPIO_SET, (byte) port, (byte) pin, CavanJava.getBoolValueByte(value) };
		return mCommand.readBool(command);
	}

	public boolean doConfigGpio(int port, int pin, int mode, int function, boolean high) throws Exception {
		byte[] command = { JWAOO_TOY_CMD_GPIO_CFG, (byte) port, (byte) pin, (byte) mode, (byte) function, CavanJava.getBoolValueByte(high) };
		return mCommand.readBool(command);
	}

	public boolean setLedEnable(int index, boolean enable) throws Exception {
		byte[] command = { JWAOO_TOY_CMD_LED_ENABLE, (byte) index, CavanJava.getBoolValueByte(enable) };
		return mCommand.readBool(command);
	}

	public boolean setBtLedEnable(boolean enable) throws Exception {
		return setLedEnable(LED_BT, enable);
	}

	public boolean setBattLedEnable(boolean enable) throws Exception {
		return setLedEnable(LED_BATT, enable);
	}

	public boolean setStateLedEnable(boolean enable) throws Exception {
		return setLedEnable(mStateLed, enable);
	}

	public boolean setHeaterEnable(boolean enable) throws Exception {
		return mCommand.readBool(JWAOO_TOY_CMD_HEATER_ENABLE, enable);
	}

	public boolean isHeaterEnabled() throws Exception {
		return mCommand.readBool(JWAOO_TOY_CMD_HEATER_ENABLE);
	}

	public JwaooToyTestResult readTestResult() throws Exception {
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

	public boolean writeTestResult(JwaooToyTestResult result) throws Exception {
		mCommand.readBool(result.buildCommand());
		return true;
	}

	public JwaooToyAppSettings readAppSettings() throws Exception {
		JwaooToyResponse response = mCommand.send(JWAOO_TOY_CMD_APP_SETTINGS);
		if (response == null) {
			return null;
		}

		return response.getAppSettings();
	}

	public int getVersion() throws Exception {
		if (mVersion == 0) {
			mVersion = readVersion();
		}

		return mVersion;
	}

	public String getVersionString() throws Exception {
		return Integer.toHexString(getVersion());
	}

	public String getBuildDateString() throws Exception {
		if (mBuildDate == null) {
			mBuildDate = readBuildDate();
		}

		return mBuildDate;
	}

	@SuppressWarnings("deprecation")
	public Date getBuildDate() throws Exception {
		String text = getBuildDateString();
		if (text == null) {
			return null;
		}

		return new Date(text);
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

	public boolean getKeyLockState() throws Exception {
		return mCommand.readValue8(JWAOO_TOY_CMD_KEY_LOCK, (byte) 0) > 0;
	}

	public boolean setKeyLock(boolean enable) throws Exception {
		return mCommand.readBool(JWAOO_TOY_CMD_KEY_LOCK, enable);
	}

	public boolean setKeyReportEnable(int mask) throws Exception {
		return mCommand.readBool(JWAOO_TOY_CMD_KEY_REPORT_ENABLE, (byte) mask);
	}

	public boolean writeAppData(byte[] bytes) throws Exception {
		byte[] command = new byte[bytes.length + 1];

		command[0] = JWAOO_TOY_CMD_APP_DATA;
		CavanArray.copy(bytes, 0, command, 1, bytes.length);

		return mCommand.readBool(command);
	}

	public byte[] readAppData() throws Exception {
		return mCommand.readData(JWAOO_TOY_CMD_APP_DATA);
	}

	public boolean setSuspendDelay(int delay) throws Exception {
		return mCommand.readBool(JWAOO_TOY_CMD_SUSPEND_DELAY, (short) delay);
	}

	public int getSuspendDelay() throws Exception {
		return mCommand.readValue16(JWAOO_TOY_CMD_SUSPEND_DELAY, (short) -1);
	}

	public boolean setShutdownVoltage(int voltage) throws Exception {
		return mCommand.readBool(JWAOO_TOY_CMD_BATT_SHUTDOWN_VOLTAGE, (short) voltage);
	}

	public int getShutdownVoltage() throws Exception {
		return mCommand.readValue16(JWAOO_TOY_CMD_BATT_SHUTDOWN_VOLTAGE, (short) -1);
	}

	public JwaooToyBatteryInfo getBatteryInfo() throws Exception {
		JwaooToyResponse response = mCommand.send(JWAOO_TOY_CMD_BATT_INFO);
		if (response == null) {
			return null;
		}

		return response.getBatteryInfo();
	}

	@Override
	protected boolean doInitialize() {
		mVersion = 0;
		mBuildDate = null;
		mVoltageRatio = 1;

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

		try {
			if (!mCharEvent.setDataListener(mEventDataListener)) {
				CavanAndroid.eLog("Failed to mCharEvent.setDataListener");
				return false;
			}

			if (!mCharSensor.setDataListener(mSensorDataListener)) {
				CavanAndroid.eLog("Failed to mCharSensor.setDataListener");
				return false;
			}
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}

		setGattReady(true);

		String identify;

		try {
			identify = doIdentify();
			if (identify == null) {
				CavanAndroid.eLog("Failed to doIdentify");
				return false;
			}
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}

		CavanAndroid.dLog("identify = " + identify);

		mStateLed = LED_BT;

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
		} else if (identify.equals(DEVICE_NAME_MODEL11)) {
			mSensor = new JwaooToySensorModel10();
			mDeviceName = DEVICE_NAME_MODEL11;
			mDeviceId = DEVICE_ID_MODEL11;
		} else if (identify.equals(DEVICE_NAME_MODEL01)) {
			mSensor = new JwaooToySensorModel10();
			mDeviceName = DEVICE_NAME_MODEL01;
			mDeviceId = DEVICE_ID_MODEL01;
			mStateLed = LED_BATT;
		} else if (identify.equals(DEVICE_NAME_MODEL03)) {
			mSensor = new JwaooToySensorModel10();
			mDeviceName = DEVICE_NAME_MODEL03;
			mDeviceId = DEVICE_ID_MODEL03;
		} else {
			CavanAndroid.eLog("Invalid identify");
			return false;
		}

		return true;
	}

	// ================================================================================

	public class JwaooToyBatteryInfo {

		private int mState;
		private int mLevel;
		private double mVoltage;
		private double mVoltageNtc;

		private JwaooToyBatteryInfo(byte[] bytes, int offset) {
			CavanByteCache cache = new CavanByteCache(bytes);
			cache.setOffset(offset);

			mState = cache.readValue8();
			mLevel = cache.readValue8();
			mVoltage = ((double) cache.readValue16()) / 1000;
			mVoltageNtc = ((double) cache.readValue16()) / 1000;
		}

		public int getState() {
			return mState;
		}

		public double getVoltage() {
			return mVoltage;
		}

		public double getVoltageNtc() {
			return mVoltageNtc;
		}

		public int getLevel() {
			return mLevel;
		}

		public double getTemp() {
			return 84.296 - (mVoltageNtc * 9.497);
		}

		public int getLevelByVoltage(double voltage) {
			int level = mBatteryCapacityTable.getCapacityInt(voltage);
			if (level > 99 && mState == BATTERY_STATE_CHARGING) {
				return 99;
			}

			return level;
		}

		public int getLevelByVoltage() {
			return getLevelByVoltage(mVoltage);
		}

		public String getStateString() {
			switch (mState) {
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

		public void calibration(double voltage) {
			CavanAndroid.dLog("voltage = " + voltage + ", mVoltage = " + mVoltage);

			if (mVoltage > 0) {
				mVoltageRatio = voltage / mVoltage;
			}

			CavanAndroid.dLog("mVoltageRatio = " + mVoltageRatio);
		}

		public double getFixedVoltage() {
			return mVoltage * mVoltageRatio;
		}

		public double getFixedLevel() {
			return getLevelByVoltage(getFixedVoltage());
		}

		@Override
		public String toString() {
			StringBuilder builder = new StringBuilder();

			builder.append("voltage:").append(mVoltage);
			builder.append(", ntc:").append(mVoltageNtc);
			builder.append(", level:").append(getLevel());
			builder.append(", state:").append(getStateString());

			return builder.toString();
		}
	}

	public class JwaooToyAppSettings {

		public static final int COMMAND_LENGTH = 15;

		private int mSuspendDelay;
		private int mShutdownVoltage;
		private int mBtLedCloseTime;
		private int mBtLedOpenTime;
		private int mMotoRandMax;
		private int mMotoRandDelay;
		private int mMotoSpeedMin;
		private int mBattLevelLow;
		private int mBattLevelNormal;
		private int mBattPollDelay;
		private int mBattFlags;

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
			mBattLevelLow = cache.readValue8();
			mBattLevelNormal = cache.readValue8();
			mBattPollDelay = cache.readValue8();
			mBattFlags = cache.readValue8();
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
			cache.writeValue8((byte) mBattLevelLow);
			cache.writeValue8((byte) mBattLevelNormal);
			cache.writeValue8((byte) mBattPollDelay);
			cache.writeValue8((byte) mBattFlags);
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

		public int getBattLevelLow() {
			return mBattLevelLow;
		}

		public void setBattLevelLow(int level) {
			mBattLevelLow = level;
		}

		public int getBattLevelNormal() {
			return mBattLevelNormal;
		}

		public void setBattLevelNormal(int level) {
			mBattLevelNormal = level;
		}

		public int getBattPollDelay() {
			return mBattPollDelay;
		}

		public void setBattPollDelay(int delay) {
			mBattPollDelay = delay;
		}

		public int getBattFlags() {
			return mBattFlags;
		}

		public void setBattFlags(int flags) {
			mBattFlags = flags;
		}

		public boolean commit() throws Exception {
			return mCommand.readBool(buildCommand());
		}

		@Override
		public String toString() {
			StringBuilder builder = new StringBuilder();

			builder.append("suspend_delay:").append(mSuspendDelay);
			builder.append(", shutdown_voltage:").append(mShutdownVoltage);
			builder.append(", bt_led_open_time:").append(mBtLedOpenTime);
			builder.append(", bt_led_close_time:").append(mBtLedCloseTime);
			builder.append(", moto_rand_delay:").append(mMotoRandDelay);
			builder.append(", moto_rand_max:").append(mMotoRandMax);
			builder.append(", moto_speed_min:").append(mMotoSpeedMin);
			builder.append(", batt_level_low:").append(mBattLevelLow);
			builder.append(", batt_level_normal:").append(mBattLevelNormal);
			builder.append(", batt_poll_delay:").append(mBattPollDelay);
			builder.append(", batt_flags:").append(Integer.toHexString(mBattFlags));

			return builder.toString();
		}
	}

	public class JwaooToyKeySettings {

		public static final int COMMAND_LENGTH = 10;

		private boolean mClickEnable;
		private boolean mMultiClickEnable;
		private int mMultiClickDelay;
		private boolean mLongClickEnable;
		private int mLongClickDelay;
		private int mLedBlinkDelay;

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

		public void setLedBlinkDelay(int delay) {
			mLedBlinkDelay = delay;
		}

		public int getLedBlinkDelay() {
			return mLedBlinkDelay;
		}

		public boolean commit() throws Exception {
			return mCommand.readBool(buildCommand());
		}

		@Override
		public String toString() {
			StringBuilder builder = new StringBuilder();

			builder.append("click_enable:").append(mClickEnable);
			builder.append(", multi_click_enable:").append(mMultiClickEnable);
			builder.append(", multi_click_delay:").append(mMultiClickDelay);
			builder.append(", long_click_enable:").append(mLongClickEnable);
			builder.append(", long_click_delay:").append(mLongClickDelay);
			builder.append(", blink_delay:").append(mLedBlinkDelay);

			return builder.toString();
		}
	}

	// ================================================================================

	public class JwaooToyMotoMode {

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

		public boolean commit() throws Exception {
			return setMotoMode(mMode, mLevel);
		}

		@Override
		public String toString() {
			return "mode = " + mMode + ", level = " + mLevel;
		}
	}

	public class JwaooToyTestResult {

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

	public class JwaooToyResponse {

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

		public short getValueU8(short defValue) {
			if (getType() != JWAOO_TOY_RSP_U8 || length() != 3) {
				return defValue;
			}

			return (short) (mBytes[2] & 0xFF);
		}

		public short getValue16(short defValue) {
			if (getType() != JWAOO_TOY_RSP_U16 || length() != 4) {
				return defValue;
			}

			return CavanJava.buildValue16(mBytes, 2);
		}

		public int getValueU16(int defValue) {
			if (getType() != JWAOO_TOY_RSP_U16 || length() != 4) {
				return defValue;
			}

			return CavanJava.buildValueU16(mBytes, 2);
		}

		public int getValue32(int defValue) {
			if (getType() != JWAOO_TOY_RSP_U32 || length() != 6) {
				return defValue;
			}

			return CavanJava.buildValue32(mBytes, 2);
		}

		public long getValueU32(long defValue) {
			if (getType() != JWAOO_TOY_RSP_U32 || length() != 6) {
				return defValue;
			}

			return CavanJava.buildValueU32(mBytes, 2);
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

			return CavanArray.clone(mBytes, 2);
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

			return new JwaooToyAppSettings(mBytes);
		}

		public JwaooToyKeySettings getKeySettings() {
			if (getType() != JWAOO_TOY_RSP_DATA) {
				return null;
			}

			return new JwaooToyKeySettings(mBytes);
		}

		public JwaooToyBatteryInfo getBatteryInfo() {
			if (getType() != JWAOO_TOY_RSP_DATA) {
				return null;
			}

			return new JwaooToyBatteryInfo(mBytes, 2);
		}
	}

	// ================================================================================

	public class JwaooToyCommand {

		synchronized public JwaooToyResponse send(byte[] command) throws Exception {
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

		public JwaooToyResponse send(byte type) throws Exception {
			return send(new byte[] { type });
		}

		public byte[] buildCommand(byte type, byte[] data) {
			byte[] command = new byte[data.length + 1];

			command[0] = type;
			CavanArray.copy(data, 0, command, 1, data.length);

			return command;
		}

		public JwaooToyResponse send(byte type, byte[] data) throws Exception {
			return send(buildCommand(type, data));
		}

		public JwaooToyResponse send(byte type, String text) throws Exception {
			return send(type, text.getBytes());
		}

		public JwaooToyResponse send(byte type, byte value) throws Exception {
			byte[] command = { type, value };
			return send(command);
		}

		public JwaooToyResponse send(byte type, short value) throws Exception {
			CavanByteCache cache = new CavanByteCache(3);

			cache.writeValue8(type);
			cache.writeValue16(value);

			return send(cache.getBytes());
		}

		public JwaooToyResponse send(byte type, int value) throws Exception {
			CavanByteCache cache = new CavanByteCache(5);

			cache.writeValue8(type);
			cache.writeValue32(value);

			return send(cache.getBytes());
		}

		public JwaooToyResponse send(byte type, boolean value) throws Exception {
			return send(type, CavanJava.getBoolValueByte(value));
		}

		public JwaooToyResponse send(byte type, boolean enable, byte value) throws Exception {
			byte[] command = { type, CavanJava.getBoolValueByte(enable) , value };
			return send(command);
		}

		public JwaooToyResponse send(byte type, boolean enable, short value) throws Exception {
			CavanByteCache cache = new CavanByteCache(4);

			cache.writeValue8(type);
			cache.writeBool(enable);
			cache.writeValue16(value);

			return send(cache.getBytes());
		}

		public JwaooToyResponse send(byte type, boolean enable, int value) throws Exception {
			CavanByteCache cache = new CavanByteCache(6);

			cache.writeValue8(type);
			cache.writeBool(enable);
			cache.writeValue32(value);

			return send(cache.getBytes());
		}

		public boolean readBool(byte[] command) throws Exception {
			return getBool(send(command));
		}

		public boolean readBool(byte type) throws Exception {
			return getBool(send(type));
		}

		public boolean readBool(byte type, byte value) throws Exception {
			return getBool(send(type, value));
		}

		public boolean readBool(byte type, short value) throws Exception {
			return getBool(send(type, value));
		}

		public boolean readBool(byte type, int value) throws Exception {
			return getBool(send(type, value));
		}

		public boolean readBool(byte type, String text) throws Exception {
			return getBool(send(type, text));
		}

		public boolean readBool(byte type, byte[] data) throws Exception {
			return getBool(send(type, data));
		}

		public boolean readBool(byte type, boolean enable) throws Exception {
			return getBool(send(type, enable));
		}

		public boolean readBool(byte type, boolean enable, byte value) throws Exception {
			return getBool(send(type, enable, value));
		}

		public boolean readBool(byte type, boolean enable, short value) throws Exception {
			return getBool(send(type, enable, value));
		}

		public boolean readBool(byte type, boolean enable, int value) throws Exception {
			return getBool(send(type, enable, value));
		}

		public byte readValue8(byte[] command, byte defValue) throws Exception {
			return getValue8(send(command), defValue);
		}

		public byte readValue8(byte type, byte defValue) throws Exception {
			return getValue8(send(type), defValue);
		}

		public short readValue16(byte type, short defValue) throws Exception {
			return getValue16(send(type), defValue);
		}

		public int readValue32(byte type, int defValue) throws Exception {
			return getValue32(send(type), defValue);
		}

		public String readText(byte type) throws Exception {
			return getText(send(type));
		}

		public byte[] readData(byte[] command) throws Exception {
			return getData(send(command));
		}

		public byte[] readData(byte type) throws Exception {
			return getData(send(type));
		}

		public boolean readArray16(byte[] command, short[] array, int offset, int count) throws Exception {
			return getArray16(send(command), array, offset, count);
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

			try {
				return mCommand.readData(command);
			} catch (Exception e) {
				e.printStackTrace();
				return null;
			}
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
	}
}
