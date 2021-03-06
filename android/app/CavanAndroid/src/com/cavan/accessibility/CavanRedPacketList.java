package com.cavan.accessibility;

import java.util.Iterator;

import com.cavan.android.CavanAndroid;

public class CavanRedPacketList implements Iterable<CavanRedPacket> {

	private CavanRedPacket mHead = new CavanRedPacket();

	public synchronized boolean add(CavanAccessibilityPackage pkg, CavanRedPacket packet) {
		CavanAndroid.dLog("add: " + packet);
		if (packet == null || packet.isCompleted()) {
			return false;
		}

		CavanRedPacket head;

		for (head = mHead.next; head != mHead; head = head.next) {
			if (head.equals(packet)) {
				return false;
			}

			if (head.getUnpackTime() > packet.getUnpackTime()) {
				for (CavanRedPacket node = head.next; node != mHead; node = node.next) {
					if (node.equals(packet)) {
						return false;
					}
				}
				break;
			}
		}

		head.addPrev(pkg, packet);
		packet.onAdded();

		return true;
	}

	public synchronized void remove(CavanRedPacket packet) {
		CavanAndroid.dLog("remove: " + packet);
		if (packet != null) {
			packet.remove();
			packet.onRemoved();
		}
	}

	public synchronized int remove(CavanAccessibilityPackage pkg) {
		CavanRedPacket node = mHead.next;
		int count = 0;

		while (node != mHead) {
			CavanRedPacket next = node.next;

			if (node.getPackage() == pkg) {
				remove(node);
				count++;
			}

			node = next;
		}

		return count;
	}

	public synchronized boolean isEmpty() {
		return (mHead.next == mHead);
	}

	public synchronized CavanRedPacket get() {
		CavanRedPacket packet = mHead.next;
		if (packet != mHead) {
			return packet;
		}

		return null;
	}

	public synchronized void clear() {
		CavanRedPacket packet = mHead.next;

		while (packet != mHead) {
			CavanRedPacket next = packet.next;
			remove(packet);
			packet = next;
		}

		mHead.clear();
	}

	@Override
	public Iterator<CavanRedPacket> iterator() {
		return new Iterator<CavanRedPacket>() {

			private CavanRedPacket mNext = mHead.next;
			private CavanRedPacket mNode;

			@Override
			public void remove() {
				if (mNode != null && mNode != mHead) {
					mNode.remove();
					mNode = null;
				}
			}

			@Override
			public CavanRedPacket next() {
				mNode = mNext;
				mNext = mNode.next;
				return mNode;
			}

			@Override
			public boolean hasNext() {
				return (mNext != mHead);
			}
		};
	}
}
