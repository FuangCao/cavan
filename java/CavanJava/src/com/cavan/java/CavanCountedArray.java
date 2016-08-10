package com.cavan.java;

import java.util.HashMap;
import java.util.Iterator;

@SuppressWarnings("serial")
public class CavanCountedArray<E> extends HashMap<E, Integer> {

	private int mCountMax;

	public CavanCountedArray(int max) {
		super();
		mCountMax = max;
	}

	public void addCountedValue(E value) {
		Integer count = get(value);
		if (count == null) {
			count = 2;
		} else {
			count += 2;

			if (count > mCountMax) {
				count = mCountMax;
			}
		}

		put(value, count);

		Iterator<java.util.Map.Entry<E, Integer>> iterator = entrySet().iterator();

		while (iterator.hasNext()) {
			java.util.Map.Entry<E, Integer> entry = iterator.next();

			if (entry.getValue() > 1) {
				entry.setValue(entry.getValue() - 1);
			} else {
				iterator.remove();
			}
		}
	}

	public E getBestValue() {
		E value = null;
		int count = 0;

		for (java.util.Map.Entry<E, Integer> entry : entrySet()) {
			if (entry.getValue() > count) {
				count = entry.getValue();
				value = entry.getKey();
			}
		}

		return value;
	}

	public E putCountedValue(E value) {
		addCountedValue(value);
		return getBestValue();
	}
}
