package com.cavan.java;

import java.io.Serializable;
import java.util.AbstractSequentialList;
import java.util.Collection;
import java.util.Deque;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;
import java.util.Queue;

public class CavanLinkedList<E> extends AbstractSequentialList<E> implements List<E>, Deque<E>, Queue<E>, Cloneable, Serializable {

	private static final long serialVersionUID = 7436083814712030101L;

	public class LinkNode {
		public E value;
		public LinkNode next;
		public LinkNode prev;

		public LinkNode(E value, LinkNode prev, LinkNode next) {
			this.value = value;
			init(prev, next);
		}

		public LinkNode(E value) {
			this.value = value;
			init();
		}

		public void init(LinkNode prev, LinkNode next) {
			this.prev = prev;
			this.next = next;
		}

		public void init() {
			init(this, this);
		}

		public void addTail(LinkNode node) {
			node.prev = this;
			node.next = next;
			next.prev = node;
			next = node;
			mSize++;
		}

		public void addHead(LinkNode node) {
			node.next = this;
			node.prev = prev;
			prev.next = node;
			prev = node;
			mSize++;
		}

		public boolean addAll(Collection<? extends E> collection) {
			LinkNode next = this.next;
			LinkNode prev = this;

			for (E value : collection) {
				LinkNode node = new LinkNode(value, prev, next);
				prev.next = node;
				prev = node;
			}

			next.prev = prev;
			mSize += collection.size();

			return true;
		}

		public void remove() {
			prev.next = next;
			next.prev = prev;
			mSize--;
		}

		public boolean isEmpty() {
			return next == this;
		}

		public void shiftLeft() {
			prev.next = next;
			next.prev = prev;
			next = prev;
			prev = prev.prev;
			next.prev = this;
			prev.next = this;
		}

		public void shiftRight() {
		}
	}

	public class LinkIterator implements ListIterator<E> {

		private int mIndex;
		private LinkNode mNode;

		public LinkIterator(int location) {
			mNode = getPrevNode(location);
			mIndex = location;
		}

		public LinkIterator() {
			mNode = mHead;
			mIndex = 0;
		}

		@Override
		public void add(E object) {
			mNode.addTail(new LinkNode(object));
		}

		@Override
		public boolean hasNext() {
			return mNode.next != mHead;
		}

		@Override
		public boolean hasPrevious() {
			return mNode.prev != mHead;
		}
		@Override
		public E next() {
			mIndex++;
			mNode = mNode.next;
			return mNode.value;
		}
		@Override
		public int nextIndex() {
			return mIndex + 1;
		}
		@Override
		public E previous() {
			mIndex--;
			mNode = mNode.prev;
			return mNode.value;
		}

		@Override
		public int previousIndex() {
			return mIndex - 1;
		}

		@Override
		public void remove() {
			LinkNode node = mNode.next;
			mNode.remove();
			mNode = node;
		}

		@Override
		public void set(E object) {
			mNode.value = object;
		}
	}

	public class ReverseLinkIterator implements Iterator<E> {

		private LinkNode mNode = mHead.prev;

		@Override
		public boolean hasNext() {
			return mNode.prev != mHead;
		}

		@Override
		public E next() {
			mNode = mNode.prev;
			return mNode.value;
		}

		@Override
		public void remove() {
			LinkNode prev = mNode.prev;
			mNode.remove();
			mNode = prev;
		}
	}

	protected int mSize;
	protected LinkNode mHead = new LinkNode(null);

	public LinkNode getNode(int location) {
		LinkNode node = mHead.next;

		while (location > 0) {
			node = node.next;
			location--;
		}

		return node;
	}

	public LinkNode getPrevNode(int location) {
		LinkNode node = mHead;

		while (location > 0) {
			node = node.next;
			location--;
		}

		return node;
	}

	public LinkNode findNode(Object object) {
		for (LinkNode node = mHead.next; node != mHead; node = node.next) {
			if (node.equals(object)) {
				return node;
			}
		}

		return null;
	}

	@Override
	public void addFirst(E e) {
		mHead.addTail(new LinkNode(e));
	}

	@Override
	public void addLast(E e) {
		mHead.addHead(new LinkNode(e));
	}

	@Override
	public boolean offerFirst(E e) {
		addFirst(e);
		return true;
	}

	@Override
	public boolean offerLast(E e) {
		addLast(e);
		return true;
	}

	@Override
	public E removeFirst() {
		LinkNode node = mHead.next;
		if (node == mHead) {
			return null;
		}

		node.remove();

		return node.value;
	}

	@Override
	public E removeLast() {
		LinkNode node = mHead.prev;
		if (node == mHead) {
			return null;
		}

		node.remove();

		return node.value;
	}

	@Override
	public E pollFirst() {
		return removeFirst();
	}

	@Override
	public E pollLast() {
		return removeLast();
	}

	@Override
	public E getFirst() {
		return mHead.next.value;
	}

	@Override
	public E getLast() {
		return mHead.prev.value;
	}

	@Override
	public E peekFirst() {
		return getFirst();
	}

	@Override
	public E peekLast() {
		return getLast();
	}

	@Override
	public boolean removeFirstOccurrence(Object o) {
		LinkNode node = mHead.next;

		while (node != mHead) {
			if (node.value.equals(o)) {
				node.remove();
				return true;
			}

			node = node.next;
		}

		return false;
	}

	@Override
	public boolean removeLastOccurrence(Object o) {
		LinkNode node = mHead.prev;

		while (node != mHead) {
			if (node.value.equals(o)) {
				node.remove();
				return true;
			}

			node = node.prev;
		}

		return false;
	}

	@Override
	public boolean offer(E e) {
		return offerLast(e);
	}

	@Override
	public E remove() {
		return removeFirst();
	}

	@Override
	public E poll() {
		return pollFirst();
	}

	@Override
	public E element() {
		return getFirst();
	}

	@Override
	public E peek() {
		return peekFirst();
	}

	@Override
	public void push(E e) {
		addFirst(e);
	}

	@Override
	public E pop() {
		return removeFirst();
	}

	@Override
	public Iterator<E> descendingIterator() {
		return new ReverseLinkIterator();
	}

	@Override
	public void add(int location, E object) {
		LinkNode node = getPrevNode(location);
		node.addTail(new LinkNode(object));
	}

	@Override
	public boolean add(E object) {
		addLast(object);
		return true;
	}

	@Override
	public boolean addAll(int location, Collection<? extends E> collection) {
		return getPrevNode(location).addAll(collection);
	}

	@Override
	public boolean addAll(Collection<? extends E> collection) {
		return mHead.prev.addAll(collection);
	}

	@Override
	public void clear() {
		mHead.init();
		mSize = 0;
	}

	@Override
	public boolean contains(Object object) {
		for (LinkNode node = mHead.next; node != mHead; node = node.next)
		{
			if (node.value.equals(object)) {
				return true;
			}
		}

		return false;
	}

	@Override
	public E get(int location) {
		LinkNode node = getNode(location);
		if (node == mHead) {
			return null;
		}

		return node.value;
	}

	@Override
	public int indexOf(Object object) {
		int location = 0;

		for (LinkNode node = mHead.next; node != mHead; node = node.next) {
			if (node.value.equals(object)) {
				return location;
			}

			location++;
		}

		return -1;
	}

	@Override
	public boolean isEmpty() {
		return mHead.isEmpty();
	}

	@Override
	public Iterator<E> iterator() {
		return new LinkIterator();
	}

	@Override
	public int lastIndexOf(Object object) {
		return mSize - 1;
	}

	@Override
	public ListIterator<E> listIterator(int location) {
		return new LinkIterator(location);
	}

	@Override
	public E remove(int location) {
		LinkNode node = getNode(location);
		if (node == mHead) {
			return null;
		}

		node.remove();

		return node.value;
	}

	@Override
	public boolean remove(Object object) {
		LinkNode node = findNode(object);
		if (node == null) {
			return false;
		}

		node.remove();

		return true;
	}

	@Override
	public E set(int location, E object) {
		LinkNode node = getNode(location);
		E value = node.value;
		node.value = object;
		return value;
	}

	@Override
	public int size() {
		return mSize;
	}

	@Override
	public Object[] toArray() {
		Object[] objects = new Object[mSize];
		int index = 0;

		for (LinkNode node = mHead.next; node != mHead; node = node.next) {
			objects[index++] = node.value;
		}

		return objects;
	}

	@SuppressWarnings("unchecked")
	@Override
	public <T> T[] toArray(T[] array) {
		int index = 0;

		for (LinkNode node = mHead.next; node != mHead && index < array.length; node = node.next) {
			array[index++] = (T) node.value;
		}

		return array;
	}
}
