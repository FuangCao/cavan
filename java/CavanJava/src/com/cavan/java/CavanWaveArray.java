package com.cavan.java;

@SuppressWarnings("serial")
public class CavanWaveArray extends CavanTimedArray<Double> {

	private boolean mNeedFind = true;
	protected CavanTimedNode<Double> mMinNode;
	protected CavanTimedNode<Double> mMaxNode;

	public CavanWaveArray(long overtime) {
		super(overtime);
	}

	protected void updateExtremeNode() {
		mMinNode = mMaxNode = get(0);

		for (int i = size() - 1; i > 0; i--) {
			CavanTimedNode<Double> node = get(i);

			if (node.getValue() > mMaxNode.getValue()) {
				mMaxNode = node;
			} else if (node.getValue() < mMinNode.getValue()) {
				mMinNode = node;
			}
		}
	}

	protected void setMaxNode(CavanTimedNode<Double> node) {
		mMaxNode = node;
	}

	protected void setMinNode(CavanTimedNode<Double> node) {
		mMinNode = node;
	}

	public CavanTimedNode<Double> getMaxNode() {
		return mMaxNode;
	}

	public CavanTimedNode<Double> getMinNode() {
		return mMinNode;
	}

	public double getMaxValue() {
		if (mMaxNode == null) {
			return 0;
		}

		return mMaxNode.getValue();
	}

	public double getMinValue() {
		if (mMinNode == null) {
			return 0;
		}

		return mMinNode.getValue();
	}

	@Override
	protected CavanTimedNode<Double> removeTimedNode(int index) {
		CavanTimedNode<Double> node = super.removeTimedNode(index);
		if (node == mMinNode || node == mMaxNode) {
			mNeedFind = true;
		}

		return node;
	}

	public void addWaveNode(CavanTimedNode<Double> node) {
		super.addTimedNode(node);

		if (mNeedFind) {
			mNeedFind = false;
			updateExtremeNode();
		} else if (node.getValue() > mMaxNode.getValue()) {
			setMaxNode(node);
		} else if (node.getValue() < mMinNode.getValue()) {
			setMinNode(node);
		}
	}

	public void addWaveValue(double value) {
		addWaveNode(new CavanTimedNode<Double>(value));
	}
}
