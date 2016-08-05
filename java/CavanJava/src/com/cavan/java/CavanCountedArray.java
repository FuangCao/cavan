package com.cavan.java;


public class CavanCountedArray<E> {

	private int mMaxCount;
	private CavanCountedNode<E>[] mNodes;

	@SuppressWarnings("unchecked")
	public CavanCountedArray(int size, int maxCount) {
		mMaxCount = maxCount;
		mNodes = (CavanCountedNode<E>[]) new CavanCountedNode<?>[size];

		for (int i = mNodes.length - 1; i >= 0; i--) {
			mNodes[i] = new CavanCountedNode<E>(null);
		}
	}

	public CavanCountedArray(int size) {
		this(size, size);
	}

	public CavanCountedNode<E> addCountedValue(E value) {
		CavanCountedNode<E> node;

		for (int i = mNodes.length - 1; i >= 0; i--) {
			node = mNodes[i];

			if (value.equals(node.getValue())) {
				while (--i >= 0) {
					mNodes[i].decrement();
				}

				node.increment(mMaxCount);

				return node;
			}

			node.decrement();
		}

		node = mNodes[0];

		if (node.getCount() > 0) {
			for (int i = mNodes.length - 1; i > 0; i--) {
				if (mNodes[i].isLessThen(node)) {
					node = mNodes[i];
				}
			}
		}

		node.setValue(value);

		return node;
	}

	public CavanCountedNode<E> getBestNode() {
		CavanCountedNode<E> node = mNodes[0];

		for (int i = mNodes.length - 1; i > 0; i--) {
			if (mNodes[i].isGreaterThen(node)) {
				node = mNodes[i];
			}
		}

		return node;
	}

	public E getBestValue() {
		CavanCountedNode<E> node = getBestNode();
		return node.getValue();
	}

	public E putCountedValue(E value) {
		addCountedValue(value);
		return getBestValue();
	}
}
