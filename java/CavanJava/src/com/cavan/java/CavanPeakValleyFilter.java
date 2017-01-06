package com.cavan.java;

import java.util.LinkedList;
import java.util.List;

public class CavanPeakValleyFilter extends CavanPeakValleyFinder {

	protected long mOverTime;
	protected double mValueFuzz;

	private CavanPeakValleyValue mRiseNode;
	private CavanPeakValleyValue mFallNode;

	protected List<CavanPeakValleyValue> mRiseNodes = new LinkedList<CavanPeakValleyValue>();
	protected List<CavanPeakValleyValue> mFallNodes = new LinkedList<CavanPeakValleyValue>();

	public CavanPeakValleyFilter(double fuzz, long overtime) {
		mValueFuzz = fuzz;
		mOverTime = overtime;
	}

	public void setValueFuzz(double fuzz) {
		mValueFuzz = fuzz;
	}

	public void setOverTime(long time) {
		mOverTime = time;
	}

	private void addNode(List<CavanPeakValleyValue> nodes, CavanPeakValleyValue node) {
		nodes.add(node);

		long time = node.getTime();

		while (time - nodes.get(0).getTime() > mOverTime) {
			nodes.remove(0);
		}
	}

	public CavanRangeValue getExtremeValue(List<CavanPeakValleyValue> nodes) {
		double min, max;
		CavanPeakValleyValue node;

		try {
			node = nodes.get(0);
			max = node.getPeakValue();
			min = node.getValleyValue();
		} catch (IndexOutOfBoundsException e) {
			return null;
		}

		for (int i = nodes.size() - 1; i > 0; i--) {
			node = nodes.get(i);

			if (node.getPeakValue() > max) {
				max = node.getPeakValue();
			}

			if (node.getValleyValue() < min) {
				min = node.getValleyValue();
			}
		}

		return new CavanRangeValue(min, max);
	}

	public CavanRangeValue getExtremeValueRise() {
		return getExtremeValue(mRiseNodes);
	}

	public CavanRangeValue getExtremeValueFall() {
		return getExtremeValue(mFallNodes);
	}

	public CavanRangeValue getExtremeValue(int type) {
		if (type == TYPE_RISING) {
			return getExtremeValueRise();
		} else {
			return getExtremeValueFall();
		}
	}

	public CavanRangeValue getExtremeValue() {
		return getExtremeValueRise();
	}

	private boolean addNodeRise() {
		if (mRiseNode == null) {
			mRiseNode = copyPeakValley();
		} else if (getRange() < mValueFuzz || mRiseNode.getPeakValue() - mValleyValue < mValueFuzz) {
			if (mPeakValue > mRiseNode.getPeakValue()) {
				mRiseNode.setPeakValue(mPeakValue);
				mRiseNode.setTime(mTime);
			}
		} else {
			addNode(mRiseNodes, mRiseNode);
			mRiseNode = copyPeakValley();
			return true;
		}

		return false;
	}

	private boolean addNodeFall() {
		if (mFallNode == null) {
			mFallNode = copyPeakValley();
		} else if (mPeakValue - mFallNode.getValleyValue() < mValueFuzz) {
			mFallNode.setValleyValue(mValleyValue);
			mFallNode.setTime(mTime);
		} else {
			addNode(mFallNodes, mFallNode);
			mFallNode = copyPeakValley();
			return true;
		}

		return false;
	}

	@Override
	public boolean putValue(double value) {
		if (super.putValue(value)) {
			if (isRising()) {
				return addNodeRise();
			} else {
				return addNodeFall();
			}
		}

		return false;
	}

	public boolean putValueRise(double value) {
		if (super.putValue(value) && isRising()) {
			return addNodeRise();
		}

		return false;
	}

	public boolean putValueFall(double value) {
		if (super.putValue(value) && isFalling()) {
			return addNodeFall();
		}

		return false;
	}
}
