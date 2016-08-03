package com.cavan.java;

public class CavanLinkNode<E> {

	private E mValue;
	private CavanLinkNode<E> mPrevNode;
	private CavanLinkNode<E> mNextNode;

	public CavanLinkNode(E value, CavanLinkNode<E> prev, CavanLinkNode<E> next) {
		mValue = value;
		mPrevNode = prev;
		mNextNode = next;
	}

	public CavanLinkNode(E value) {
		mValue = value;
		mNextNode = this;
		mPrevNode = this;
	}

	public E getValue() {
		return mValue;
	}

	public void setValue(E value) {
		mValue = value;
	}

	public CavanLinkNode<E> getNextNode() {
		return mNextNode;
	}

	public void setNextNode(CavanLinkNode<E> node) {
		mNextNode = node;
	}

	public CavanLinkNode<E> getPrevNode() {
		return mPrevNode;
	}

	public void setPrevNode(CavanLinkNode<E> node) {
		mPrevNode = node;
	}

	public CavanLinkNode<E> getNextNode(int index) {
		CavanLinkNode<E> node = this;

		while (index > 0) {
			node = node.getNextNode();
			if (node == this) {
				return null;
			}

			index--;
		}

		return node;
	}

	public CavanLinkNode<E> getPrevNode(int index) {
		CavanLinkNode<E> node = this;

		while (index > 0) {
			node = node.getPrevNode();
			if (node == this) {
				return null;
			}

			index--;
		}

		return node;
	}

	public CavanLinkNode<E> getNode(int index) {
		if (index < 0) {
			return getPrevNode(-index);
		}

		return getNextNode(index);
	}

	public void remove() {
		mNextNode.setPrevNode(mPrevNode);
		mPrevNode.setNextNode(mNextNode);
	}

	public void append(CavanLinkNode<E> node) {
		node.setNextNode(mNextNode);
		node.setPrevNode(this);

		mNextNode.setPrevNode(node);
		mNextNode = node;
	}

	public void append(E object) {
		append(new CavanLinkNode<E>(object));
	}

	public void insert(CavanLinkNode<E> node) {
		node.setNextNode(this);
		node.setPrevNode(mPrevNode);

		mPrevNode.setNextNode(node);
		mPrevNode = node;
	}

	public void insert(E value ) {
		insert(new CavanLinkNode<E>(value));
	}

	public void appendLink(CavanLinkNode<E> head) {
		CavanLinkNode<E> tail = head.getPrevNode();

		head.setPrevNode(this);
		tail.setNextNode(mNextNode);
		mNextNode.setPrevNode(tail);
		mNextNode = head;
	}

	public void insertLink(CavanLinkNode<E> head) {
		CavanLinkNode<E> tail = head.getPrevNode();

		head.setPrevNode(mPrevNode);
		tail.setNextNode(this);

		mPrevNode.setNextNode(head);
		mPrevNode = tail;
	}

	public boolean contains(Object value) {
		CavanLinkNode<E> node = this;

		while (node.getValue() != value) {
			node = node.getNextNode();
			if (node == this) {
				return false;
			}
		}

		return true;
	}

	public int indexOf(Object object) {
		int index = 0;

		for (CavanLinkNode<E> node = this; node.getValue() != object; index++) {
			node = node.getNextNode();
			if (node == this) {
				return -1;
			}
		}

		return index;
	}

	public int lastIndexOf(Object object) {
		int index = 0;
		int lastIndex = -1;
		CavanLinkNode<E> node = this;

		while (true) {
			if (node.getValue() == object) {
				lastIndex = index;
			}

			node = node.getNextNode();
			if (node == this) {
				return lastIndex;
			}

			index++;
		}
	}

	public CavanLinkNode<E> find(Object object) {
		CavanLinkNode<E> node = this;

		while (node.getValue() != object) {
			node = node.getNextNode();
			if (node == this) {
				return null;
			}
		}

		return node;
	}
}
