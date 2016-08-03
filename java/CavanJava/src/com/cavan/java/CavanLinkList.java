package com.cavan.java;

import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;

public class CavanLinkList<E> implements List<E> {

	private int mCount;
	private CavanLinkNode<E> mHead;

	@Override
	public void add(int location, E object) {
		if (mHead == null) {
			mHead = new CavanLinkNode<E>(object);
		} else {
			CavanLinkNode<E> node = mHead.getNode(location);
			if (node == null) {
				node = mHead;
			}

			node.insert(object);

			if (location == 0) {
				mHead = node;
			}
		}

		mCount++;
	}

	@Override
	public boolean add(E object) {
		if (mHead == null) {
			mHead = new CavanLinkNode<E>(object);
		} else {
			mHead.append(object);
		}

		mCount++;

		return true;
	}

	public CavanLinkNode<E> buildLink(Collection<? extends E> collection) {
		CavanLinkNode<E> head = null;

		for (E value : collection) {
			CavanLinkNode<E> node = new CavanLinkNode<E>(value, head, null);
			head = node;
		}

		if (head != null) {
			CavanLinkNode<E> tail = head;

			while (true) {
				CavanLinkNode<E> node = head.getPrevNode();
				if (node == null) {
					break;
				}

				node.setNextNode(head);
				head = node;
			}

			head.setPrevNode(tail);
			tail.setNextNode(head);
		}

		return head;
	}

	@Override
	public boolean addAll(int location, Collection<? extends E> collection) {
		CavanLinkNode<E> head = buildLink(collection);
		if (head == null) {
			return false;
		}

		if (mHead == null) {
			mHead = head;
		} else {
			CavanLinkNode<E> node = mHead.getNode(location);
			if (node == null) {
				node = mHead;
			}

			node.insertLink(head);
		}

		mCount += collection.size();

		return true;
	}

	@Override
	public boolean addAll(Collection<? extends E> collection) {
		CavanLinkNode<E> head = buildLink(collection);
		if (head == null) {
			return false;
		}

		if (mHead == null) {
			mHead = head;
		} else {
			mHead.insertLink(head);
		}

		return true;
	}

	@Override
	public void clear() {
		while (mHead != null) {
			CavanLinkNode<E> next = mHead.getNextNode();

			mHead.setNextNode(null);
			mHead.setPrevNode(null);

			mHead = next;
		}

		mCount = 0;
	}

	@Override
	public boolean contains(Object object) {
		if (mHead == null) {
			return false;
		}

		return mHead.contains(object);
	}

	@Override
	public boolean containsAll(Collection<?> collection) {
		if (mHead == null) {
			return false;
		}

		for (Object value : collection) {
			if (mHead.contains(value)) {
				return true;
			}
		}

		return false;
	}

	@Override
	public E get(int location) {
		if (mHead != null) {
			CavanLinkNode<E> node = mHead.getNode(location);
			if (node != null) {
				return node.getValue();
			}
		}

		return null;
	}

	@Override
	public int indexOf(Object object) {
		if (mHead == null) {
			return -1;
		}

		return mHead.indexOf(object);
	}

	@Override
	public boolean isEmpty() {
		return mHead == null;
	}

	@Override
	public Iterator<E> iterator() {
		return null;
	}

	@Override
	public int lastIndexOf(Object object) {
		if (mHead == null) {
			return -1;
		}

		return mHead.lastIndexOf(object);
	}

	@Override
	public ListIterator<E> listIterator() {
		return null;
	}

	@Override
	public ListIterator<E> listIterator(int location) {
		return null;
	}

	public void remove(CavanLinkNode<E> node) {
		if (node == mHead) {
			if (mCount > 1) {
				mHead = node.getNextNode();
			} else {
				mHead = null;
			}
		}

		node.remove();
		mCount--;
	}

	@Override
	public E remove(int location) {
		if (mHead == null) {
			return null;
		}

		CavanLinkNode<E> node = mHead.getNode(location);
		if (node == null) {
			return null;
		}

		remove(node);

		return node.getValue();
	}

	@Override
	public boolean remove(Object object) {
		if (mHead == null) {
			return false;
		}

		CavanLinkNode<E> node = mHead.find(object);
		if (node == null) {
			return false;
		}

		remove(node);

		return true;
	}

	@Override
	public boolean removeAll(Collection<?> collection) {
		if (mHead == null) {
			return false;
		}

		int found = 0;

		for (Object value : collection) {
			CavanLinkNode<E> node = mHead.find(value);
			if (node != null) {
				remove(node);
				found++;
			}
		}

		return found > 0;
	}

	@Override
	public boolean retainAll(Collection<?> collection) {
		if (mHead == null) {
			return false;
		}

		int count = 0;
		CavanLinkNode<E> head = null;

		for (Object value : collection) {
			CavanLinkNode<E> node = mHead.find(value);
			if (node != null) {
				remove(node);

				node.setPrevNode(head);
				head = node;
				count++;
			}
		}

		if (head != null) {
			CavanLinkNode<E> tail = head;

			while (true) {
				CavanLinkNode<E> node = head.getPrevNode();
				if (node == null) {
					break;
				}

				node.setNextNode(head);
				head = node;
			}

			tail.setNextNode(head);
			head.setPrevNode(tail);
		}

		mHead = head;
		mCount = count;

		return count > 0;
	}

	@Override
	public E set(int location, E object) {
		if (mHead == null) {
			return null;
		}

		CavanLinkNode<E> node = mHead.getNode(location);
		if (node == null) {
			return null;
		}

		E value = node.getValue();
		node.setValue(object);

		return value;
	}

	@Override
	public int size() {
		return mCount;
	}

	@Override
	public List<E> subList(int start, int end) {
		return null;
	}

	@Override
	public Object[] toArray() {
		return null;
	}

	@Override
	public <T> T[] toArray(T[] array) {
		return null;
	}
}
