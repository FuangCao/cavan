package com.cavan.java;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

public class CavanBluetoothScanRecord {

	public static final UUID BASE_UUID = UUID.fromString("00000000-0000-1000-8000-00805f9b34fb");

	/// Flag
	public static final int GAP_AD_TYPE_FLAGS = 0x01;
	/// Use of more than 16 bits UUID
	public static final int GAP_AD_TYPE_MORE_16_BIT_UUID = 0x02;
	/// Complete list of 16 bit UUID
	public static final int GAP_AD_TYPE_COMPLETE_LIST_16_BIT_UUID = 0x03;
	/// Use of more than 32 bit UUD
	public static final int GAP_AD_TYPE_MORE_32_BIT_UUID = 0x04;
	/// Complete list of 32 bit UUID
	public static final int GAP_AD_TYPE_COMPLETE_LIST_32_BIT_UUID = 0x05;
	/// Use of more than 128 bit UUID
	public static final int GAP_AD_TYPE_MORE_128_BIT_UUID = 0x06;
	/// Complete list of 128 bit UUID
	public static final int GAP_AD_TYPE_COMPLETE_LIST_128_BIT_UUID = 0x07;
	/// Shortened device name
	public static final int GAP_AD_TYPE_SHORTENED_NAME = 0x08;
	/// Complete device name
	public static final int GAP_AD_TYPE_COMPLETE_NAME = 0x09;
	/// Transmit power
	public static final int GAP_AD_TYPE_TRANSMIT_POWER = 0x0A;
	/// Class of device
	public static final int GAP_AD_TYPE_CLASS_OF_DEVICE = 0x0D;
	/// Simple Pairing Hash C
	public static final int GAP_AD_TYPE_SP_HASH_C = 0x0E;
	/// Simple Pairing Randomizer
	public static final int GAP_AD_TYPE_SP_RANDOMIZER_R = 0x0F;
	/// Temporary key value
	public static final int GAP_AD_TYPE_TK_VALUE = 0x10;
	/// Out of Band Flag
	public static final int GAP_AD_TYPE_OOB_FLAGS = 0x11;
	/// Slave connection interval range
	public static final int GAP_AD_TYPE_SLAVE_CONN_INT_RANGE = 0x12;
	/// Require 16 bit service UUID
	public static final int GAP_AD_TYPE_RQRD_16_BIT_SVC_UUID = 0x14;
	/// Require 32 bit service UUID
	public static final int GAP_AD_TYPE_RQRD_32_BIT_SVC_UUID = 0x1F;
	/// Require 128 bit service UUID
	public static final int GAP_AD_TYPE_RQRD_128_BIT_SVC_UUID = 0x15;
	/// Service data 16-bit UUID
	public static final int GAP_AD_TYPE_SERVICE_16_BIT_DATA = 0x16;
	/// Service data 32-bit UUID
	public static final int GAP_AD_TYPE_SERVICE_32_BIT_DATA = 0x20;
	/// Service data 128-bit UUID
	public static final int GAP_AD_TYPE_SERVICE_128_BIT_DATA = 0x21;
	/// Public Target Address
	public static final int GAP_AD_TYPE_PUB_TGT_ADDR = 0x17;
	/// Random Target Address
	public static final int GAP_AD_TYPE_RAND_TGT_ADDR = 0x18;
	/// Appearance
	public static final int GAP_AD_TYPE_APPEARANCE = 0x19;
	/// Advertising Interval
	public static final int GAP_AD_TYPE_ADV_INTV = 0x1A;
	/// LE Bluetooth Device Address
	public static final int GAP_AD_TYPE_LE_BT_ADDR = 0x1B;
	/// LE Role
	public static final int GAP_AD_TYPE_LE_ROLE = 0x1C;
	/// Simple Pairing Hash C-256
	public static final int GAP_AD_TYPE_SPAIR_HASH = 0x1D;
	/// Simple Pairing Randomizer R-256
	public static final int GAP_AD_TYPE_SPAIR_RAND = 0x1E;
	/// 3D Information Data
	public static final int GAP_AD_TYPE_3D_INFO = 0x3D;
	/// Manufacturer specific data
	public static final int GAP_AD_TYPE_MANU_SPECIFIC_DATA = 0xFF;

	private byte mFlags;
	private List<UUID> mUuids = new ArrayList<UUID>();

	public static UUID buildUuidShort(long value) {
		return new UUID(BASE_UUID.getMostSignificantBits() | value << 32, BASE_UUID.getLeastSignificantBits());
	}

	public CavanBluetoothScanRecord(byte[] bytes) {
		CavanByteCache cache = new CavanByteCache(bytes);
		
		while (true) {
			int length = cache.readValue8();
			if (length == 0) {
				break;
			}

			int type = cache.readValue8();

			switch (type) {
			case GAP_AD_TYPE_FLAGS:
				if (length < 2) {
					break;
				}

				mFlags = cache.readValue8();
				length--;
				break;

			case GAP_AD_TYPE_MORE_16_BIT_UUID:
			case GAP_AD_TYPE_COMPLETE_LIST_16_BIT_UUID:
				if (length < 3) {
					break;
				}

				mUuids.add(buildUuidShort(cache.readValue16()));
				length -= 2;
				break;

			case GAP_AD_TYPE_MORE_32_BIT_UUID:
			case GAP_AD_TYPE_COMPLETE_LIST_32_BIT_UUID:
				if (length < 5) {
					break;
				}

				mUuids.add(buildUuidShort(cache.readValue32()));
				length -= 4;
				break;

			case GAP_AD_TYPE_MORE_128_BIT_UUID:
			case GAP_AD_TYPE_COMPLETE_LIST_128_BIT_UUID:
				if (length < 17) {
					break;
				}

				long lsb = cache.readValue64();
				long msb = cache.readValue64();
				mUuids.add(new UUID(msb, lsb));
				length -= 16;
			}

			if (length > 1) {
				cache.seek(length - 1);
			}
		}
	}

	public byte getFlags() {
		return mFlags;
	}

	public List<UUID> getUuids() {
		return mUuids;
	}
}