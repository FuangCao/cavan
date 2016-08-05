package com.cavan.java;


public class CavanCountedArray<E> {

	private int mMaxCount;
	private CavanCountedNode<?>[] mNodes;

	public CavanCountedArray(int size, int maxCount) {
		mMaxCount = maxCount;
		mNodes = new CavanCountedNode<?>[size];

		for (int i = mNodes.length - 1; i >= 0; i--) {
			mNodes[i] = new CavanCountedNode<E>(null);
		}
	}

	public CavanCountedArray(int size) {
		this(size, size);
	}

	public CavanCountedNode<?> addCountedValue(E value) {
		CavanCountedNode<?> node;

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

	public CavanCountedNode<?> getBestNode() {
		CavanCountedNode<?> node = mNodes[0];

		for (int i = mNodes.length - 1; i > 0; i--) {
			if (mNodes[i].isGreaterThen(node)) {
				node = mNodes[i];
			}
		}

		return node;
	}

	@SuppressWarnings("unchecked")
	public E getBestValue() {
		CavanCountedNode<E> node = (CavanCountedNode<E>) getBestNode();
		return node.getValue();
	}

	public E putCountedValue(E value) {
		addCountedValue(value);
		return getBestValue();
	}
}
