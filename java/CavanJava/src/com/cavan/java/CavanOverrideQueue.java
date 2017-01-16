package com.cavan.java;



public class CavanOverrideQueue<E> {

	private int mHead;
	private int mTail;
	private int mSize;
	private Object[] mDataCache;

	public CavanOverrideQueue(int size) {
		mDataCache = new Object[size];
	}

	private int addIndex(int index, int value) {
		return (index + value) % mDataCache.length;
	}

	private void seekPrivate(int count) {
		mTail = addIndex(mTail, count);
	}

	private void skipPrivate(int count) {
		mHead = addIndex(mHead, count);
	}

	@SuppressWarnings("unchecked")
	synchronized private E removePrivate() {
		Object value = mDataCache[mHead];

		skipPrivate(1);
		mSize--;

		return (E) value;
	}

	synchronized public boolean add(E value) {
		if (mSize < mDataCache.length) {
			mDataCache[mTail] = value;
			seekPrivate(1);
			return true;
		}

		return false;
	}

	synchronized public boolean addOverride(E value) {
		mDataCache[mTail] = value;
		seekPrivate(1);

		if (mSize < mDataCache.length) {
			if (++mSize == 1) {
				notifyAll();
			}

			return true;
		}

		skipPrivate(1);

		return false;
	}

	@SuppressWarnings("unchecked")
	synchronized public E peek() {
		if (mSize > 0) {
			return (E) mDataCache[mHead];
		}

		return null;
	}

	synchronized public E remove() {
		if (mSize > 0) {
			return removePrivate();
		}

		return null;
	}

	synchronized public E removeBlocked() {
		while (mSize <= 0) {
			try {
				wait();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}

		return removePrivate();
	}

	synchronized public int size() {
		return mSize;
	}

	synchronized public void clear() {
		mHead = 0;
		mTail = 0;
		mSize = 0;
	}

	synchronized public String dump() {
		StringBuilder builder = new StringBuilder();

		builder.append('[');

		if (mSize > 0) {
			builder.append(mDataCache[mHead]);

			for (int head = addIndex(mHead, 1); head != mTail; head = addIndex(head, 1)) {
				builder.append(", ").append(mDataCache[head]);
			}
		}

		builder.append(']');

		return builder.toString();
	}

	public static void main(String args[]) {
		final CavanOverrideQueue<Integer> queue = new CavanOverrideQueue<Integer>(3);

		Thread thread = new Thread() {

			@Override
			public void run() {
				while (true) {
					CavanJava.dLog("value = " + queue.removeBlocked());

					try {
						Thread.sleep(500);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}
		};

		thread.start();

		long delays[] = {
			500 - 200, 500 + 200
		};

		for (int i = 0; i < 100; i++) {
			queue.addOverride(i);
			CavanJava.dLog("queue = " + queue.dump());

			try {
				Thread.sleep(delays[i & 1]);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}

		while (true) {
			try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
	}

	@Override
	public String toString() {
		StringBuilder builder = new StringBuilder();
		builder.append("[head:").append(mHead);
		builder.append(", tail:").append(mTail);
		builder.append(", size:").append(size());
		builder.append(']');
		return builder.toString();
	}
}
