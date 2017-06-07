package com.cavan.android;

import java.io.File;
import java.util.ArrayList;

import android.os.Environment;
import android.os.UserHandle;
import android.os.storage.StorageManager;

import com.cavan.java.CavanJava;

public class CavanStorageVolume {

	private Object mStorageVolume;

	public CavanStorageVolume(Object volume) {
		mStorageVolume = volume;
	}

	public static String[] getVolumePaths(StorageManager manager) {
		Object object = CavanJava.invokeMethod(manager, "getVolumePaths");
		if (object != null) {
			return (String[]) object;
		}

		return new String[] { Environment.getExternalStorageDirectory().getAbsolutePath() };
	}

	public static CavanStorageVolume[] getVolumeList(StorageManager manager) {
		Object[] objects = (Object[]) CavanJava.invokeMethod(manager, "getVolumeList");
		if (objects == null) {
			return null;
		}

		CavanStorageVolume[] volumes = new CavanStorageVolume[objects.length];

		for (int i = 0; i < objects.length; i++) {
			volumes[i] = new CavanStorageVolume(objects[i]);
		}

		return volumes;
	}

	public static ArrayList<CavanStorageVolume> getVolumeListMounted(StorageManager manager) {
		CavanStorageVolume[] volumes = getVolumeList(manager);
		if (volumes == null) {
			return null;
		}

		ArrayList<CavanStorageVolume> list = new ArrayList<CavanStorageVolume>();

		for (CavanStorageVolume volume : volumes) {
			if ("mounted".equals(volume.getState())) {
				list.add(volume);
			}
		}

		return list;
	}

	public Object invokeMethod(String name, Object valueDef) {
		Object object = CavanJava.invokeMethod(mStorageVolume, name);
		if (object != null) {
			return object;
		}

		return valueDef;
	}

	public Object invokeMethod(String name) {
		return invokeMethod(name, null);
	}

	public String getPath() {
		return (String) invokeMethod("getPath");
	}

	public File getPathFile() {
		return (File) invokeMethod("getPathFile");
	}

	public int getDescriptionId() {
		return (int) invokeMethod("getDescriptionId", 0);
	}

	public boolean isPrimary() {
		return (boolean) invokeMethod("isPrimary", false);
	}

	public boolean isRemovable() {
		return (boolean) invokeMethod("isRemovable", false);
	}

	public boolean isEmulated() {
		return (boolean) invokeMethod("isEmulated", false);
	}

	public int getStorageId() {
		return (int) invokeMethod("getStorageId", 0);
	}

	public int getMtpReserveSpace() {
		return (int) invokeMethod("getMtpReserveSpace", 0);
	}

	public boolean allowMassStorage() {
		return (boolean) invokeMethod("allowMassStorage", false);
	}

	public long getMaxFileSize() {
		return (long) invokeMethod("getDescriptionId", 0);
	}

	public UserHandle getOwner() {
		return (UserHandle) invokeMethod("getDescriptionId");
	}

	public int getFatVolumeId() {
		return (int) invokeMethod("getFatVolumeId", 0);
	}

	public String getState() {
		return (String) invokeMethod("getState");
	}

	@Override
	public boolean equals(Object o) {
		return mStorageVolume.equals(o);
	}

	@Override
	public int hashCode() {
		return mStorageVolume.hashCode();
	}

	@Override
	public String toString() {
		return mStorageVolume.toString();
	}
}
