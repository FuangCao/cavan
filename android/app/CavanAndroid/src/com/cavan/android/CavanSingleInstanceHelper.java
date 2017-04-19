package com.cavan.android;

public abstract class CavanSingleInstanceHelper<T> implements CavanSingleInstance<T> {

	private T mInstance;

	@Override
	synchronized public T getInstance(Object... args) {
		if (mInstance == null) {
			mInstance = createInstance(args);
		} else {
			initInstance(mInstance, args);
		}

		return mInstance;
	}

	@Override
	public void initInstance(T instance, Object... args) {
		CavanAndroid.dLog("initInstance: not implement");
	}
}