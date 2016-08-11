package com.cavan.java;

import java.util.ArrayList;
import java.util.Iterator;

@SuppressWarnings("serial")
public class CavanCountedArray<E> extends ArrayList<CavanCountedNode<E>> {

	private int mCountMax;

	public CavanCountedArray(int max) {
		super();
		mCountMax = max;
	}

	public CavanCountedNode<E> addCountedValue(E value) {
		CavanCountedNode<E> freeNode = null;
		Iterator<CavanCountedNode<E>> iterator = iterator();

		while (iterator.hasNext()) {
			CavanCountedNode<E> node = iterator.next();
			if (node.getValue().equals(value)) {
				node.increment(mCountMax);

				while (iterator.hasNext()) {
					if (iterator.next().decrement() == 0) {
						iterator.remove();
					}
				}

				return node;
			} else if (node.decrement() == 0) {
				freeNode = node;
			}
		}

		if (freeNode == null) {
			freeNode = new CavanCountedNode<E>(value);
			add(freeNode);
		} else {
			freeNode.setValue(value);
		}

		return freeNode;
	}

	public CavanCountedNode<E> getBestNode() {
		int count = 0;
		CavanCountedNode<E> targetNode = null;

		for (CavanCountedNode<E> node : this) {
			if (node.getCount() > count) {
				count = node.getCount();
				targetNode = node;
			}
		}

		return targetNode;
	}

	public E getBestValue() {
		CavanCountedNode<E> node = getBestNode();
		if (node == null) {
			return null;
		}

		return node.getValue();
	}

	public E putCountedValue(E value) {
		addCountedValue(value);
		return getBestValue();
	}
}
