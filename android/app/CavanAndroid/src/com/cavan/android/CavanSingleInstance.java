package com.cavan.android;

public interface CavanSingleInstance<T> {
	T getInstance(Object... args);
	T createInstance(Object... args);
	void initInstance(T instance, Object... args);
}