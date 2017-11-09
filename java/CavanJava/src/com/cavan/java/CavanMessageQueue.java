package com.cavan.java;

import java.security.InvalidParameterException;


public class CavanMessageQueue extends Thread {

	private static final int MAX_FREE_MESSAGES = 100;
	private static final int RUNNABLE_MSG_ID = Integer.MAX_VALUE;

	public interface CavanMessageMatcher {
		boolean match(CavanMessage message, int what, Runnable runnable);
		boolean match(CavanMessage message1, CavanMessage message2);
	};

	public class CavanMessage implements Runnable {

		public int what;
		public Object[] args;

		protected long time;
		protected Runnable runnable;
		protected CavanMessage next;

		public int getInt(int index) {
			Integer value = (Integer) args[index];
			if (value != null) {
				return value;
			}

			return 0;
		}

		public short getShort(int index) {
			Short value = (Short) args[index];
			if (value != null) {
				return value;
			}

			return 0;
		}

		public long getLong(int index) {
			Long value = (Long) args[index];
			if (value != null) {
				return value;
			}

			return 0;
		}

		public float getFloat(int index) {
			Float value = (Float) args[index];
			if (value != null) {
				return value;
			}

			return 0;
		}

		public double getDouble(int index) {
			Double value = (Double) args[index];
			if (value != null) {
				return value;
			}

			return 0;
		}

		public String getString(int index) {
			String value = (String) args[index];
			if (value != null) {
				return value;
			}

			return null;
		}

		public boolean isRunnable() {
			return runnable != this;
		}

		public boolean isNotRunnable() {
			return runnable == this;
		}

		@Override
		public void run() {
			handleMessage(this);
		}

		@Override
		public boolean equals(Object obj) {
			if (obj == null) {
				return false;
			}

			if (obj instanceof CavanMessage) {
				CavanMessage message = (CavanMessage) obj;

				if (isRunnable()) {
					return message.runnable == runnable;
				}

				return message.what == what;
			} else if (obj instanceof Runnable) {
				return runnable == obj;
			} else if (obj instanceof Integer) {
				return ((Integer) what).equals(obj);
			} else {
				return false;
			}
		}

		@Override
		public String toString() {
			StringBuilder builder = new StringBuilder();

			builder.append("what=").append(what);

			if (args != null) {
				for (int i = 0; i < args.length; i++) {
					builder.append(',').append("arg").append(i).append('=').append(args[i]);
				}
			}

			return builder.toString();
		}
	}

	private static final CavanMessageMatcher sMessageMatcherWhat = new CavanMessageMatcher() {

		@Override
		public boolean match(CavanMessage message, int what, Runnable runnable) {
			return message.what == what;
		}

		@Override
		public boolean match(CavanMessage message1, CavanMessage message2) {
			return message1.what == message2.what;
		}
	};

	private static final CavanMessageMatcher sMessageMatcherRunnable = new CavanMessageMatcher() {

		@Override
		public boolean match(CavanMessage message, int what, Runnable runnable) {
			return message.runnable == runnable;
		}

		@Override
		public boolean match(CavanMessage message1, CavanMessage message2) {
			return message1.runnable == message2.runnable;
		}
	};

	private int mFreeLength;
	private CavanMessage mHead;
	private CavanMessage mFree;

	public CavanMessageQueue() {
		super(CavanMessage.class.getName());
		start();
	}

	private synchronized void addFreeMessage(CavanMessage message) {
		if (mFreeLength < MAX_FREE_MESSAGES) {
			message.next = mFree;
			mFree = message;
			mFreeLength++;
		}
	}

	private synchronized CavanMessage getFreeMessage() {
		CavanMessage message;

		if (mFree != null) {
			mFreeLength--;
			message = mFree;
			mFree = message.next;
		} else {
			mFreeLength = 0;
			message = new CavanMessage();
		}

		return message;
	}

	public CavanMessage obtainMessage(Runnable runnable) {
		CavanMessage message = getFreeMessage();
		message.what = RUNNABLE_MSG_ID;
		message.runnable = runnable;
		return message;
	}

	public CavanMessage obtainMessage() {
		CavanMessage message = getFreeMessage();
		message.runnable = message;
		return message;
	}

	public CavanMessage obtainMessage(int what, Object... args) {
		CavanMessage message = obtainMessage();
		message.what = what;
		message.args = args;
		return message;
	}

	private synchronized CavanMessage findMessagePrev(long time) {
		CavanMessage message = mHead;
		CavanMessage prev = null;

		while (message != null && message.time <= time) {
			prev = message;
			message = message.next;
		}

		return prev;
	}

	public synchronized boolean sendMessageDelayed(CavanMessage message, long delay) {
		long time = System.currentTimeMillis() + delay;

		message.time = time;

		CavanMessage prev = findMessagePrev(time);

		if (prev == null) {
			message.next = mHead;
			mHead = message;
			notify();
		} else {
			message.next = prev.next;
			prev.next = message;
		}

		return true;
	}

	public synchronized void sendMessageDelayedUniq(CavanMessageMatcher matcher, CavanMessage message, long delay) {
		if (hasMessage(matcher, message.what, message.runnable)) {
			addFreeMessage(message);
		} else {
			sendMessageDelayed(message, delay);
		}
	}

	public synchronized void sendMessageDelayedReplace(CavanMessageMatcher matcher, CavanMessage message, long delay) {
		removeMessage(matcher, message.what, message.runnable);
		sendMessage(message);
	}

	public void sendMessage(CavanMessage message) {
		sendMessageDelayed(message, 0);
	}

	public void sendMessageDelayedUniq(CavanMessage message, long delay) {
		sendMessageDelayedUniq(sMessageMatcherWhat, message, delay);
	}

	public void sendMessageDelayedUniq(CavanMessage message) {
		sendMessageDelayedUniq(message, 0);
	}

	public void sendMessageDelayedReplace(CavanMessage message, long delay) {
		sendMessageDelayedReplace(sMessageMatcherWhat, message, delay);
	}

	public void sendMessageDelayedReplace(CavanMessage message) {
		sendMessageDelayedReplace(message, 0);
	}

	public void sendEmptyMessageDelayed(int what, long delay) {
		CavanMessage message = obtainMessage(what);
		sendMessageDelayed(message, delay);
	}

	public void sendEmptyMessage(int what) {
		sendEmptyMessageDelayed(what, 0);
	}

	public void sendEmptyMessageDelayedUniq(int what, long delay) {
		CavanMessage message = obtainMessage(what);
		sendMessageDelayedUniq(message, delay);
	}

	public void sendEmptyMessageUniq(int what) {
		sendEmptyMessageDelayedUniq(what, 0);
	}

	public void sendEmptyMessageDelayedReplace(int what, long delay) {
		CavanMessage message = obtainMessage(what);
		sendMessageDelayedReplace(message, delay);
	}

	public void sendEmptyMessageReplace(int what) {
		sendEmptyMessageDelayedReplace(what, 0);
	}

	private synchronized int removeMessage(CavanMessageMatcher matcher, int what, Runnable runnable) {
		CavanMessage message = mHead;
		CavanMessage prev = null;
		int count = 0;

		while (message != null) {
			CavanMessage next = message.next;

			if (matcher.match(message, what, runnable)) {
				if (message == mHead) {
					mHead = next;
				} else {
					prev.next = next;
				}

				addFreeMessage(message);
				count++;
			} else {
				prev = message;
			}

			message = next;
		}

		return count;
	}

	public int removeMessage(int what) {
		return removeMessage(sMessageMatcherWhat, what, null);
	}

	private boolean hasMessage(CavanMessageMatcher matcher, int what, Runnable runnable) {
		for (CavanMessage message = mHead; message != null; message = message.next) {
			if (matcher.match(message, what, runnable)) {
				return true;
			}
		}

		return false;
	}

	public boolean hasMessage(int what) {
		return hasMessage(sMessageMatcherWhat, what, null);
	}

	private synchronized int setDelay(CavanMessageMatcher matcher, int what, Runnable runnable, long delay) {
		long time = System.currentTimeMillis() + delay;
		CavanMessage message = mHead;
		CavanMessage iprev = null;
		CavanMessage prev = null;
		CavanMessage head = null;
		CavanMessage tail = null;
		int count = 0;

		while (message != null) {
			CavanMessage next = message.next;

			if (matcher.match(message, what, runnable)) {
				message.time = time;

				if (message == mHead) {
					mHead = next;
				} else {
					prev.next = next;
				}

				if (tail == null) {
					tail = message;
				}

				message.next = head;
				head = message;
				count++;
			} else {
				if (message.time <= time) {
					iprev = message;
				}

				prev = message;
			}

			message = next;
		}

		if (head == null) {
			return 0;
		}

		if (iprev == null) {
			tail.next = mHead;
			mHead = head;
			notify();
		} else {
			tail.next = iprev.next;
			iprev.next = head;
		}

		return count;
	}

	public int setDelay(int what, long delay) {
		return setDelay(sMessageMatcherWhat, what, null, delay);
	}

	public void postRunnableDelayed(Runnable runnable, long delay) {
		CavanMessage message = obtainMessage(runnable);
		sendMessageDelayed(message, delay);
	}

	public void postRunnable(Runnable runnable) {
		postRunnableDelayed(runnable, 0);
	}

	public void postRunnableDelayedUniq(Runnable runnable, long delay) {
		CavanMessage message = obtainMessage(runnable);
		sendMessageDelayedUniq(sMessageMatcherRunnable, message, delay);
	}

	public void postRunnableDelayedUniq(Runnable runnable) {
		postRunnableDelayedUniq(runnable, 0);
	}

	public void postRunnableDelayedReplace(Runnable runnable, long delay) {
		CavanMessage message = obtainMessage(runnable);
		sendMessageDelayedReplace(sMessageMatcherRunnable, message, delay);
	}

	public void postRunnableDelayedReplace(Runnable runnable) {
		postRunnableDelayedReplace(runnable, 0);
	}

	public synchronized void cancelRunable(Runnable runnable) {
		CavanMessage message = mHead;
		CavanMessage prev = null;

		while (message != null) {
			CavanMessage next = message.next;

			if (message.runnable == message) {
				if (message == mHead) {
					mHead = next;
				} else {
					prev.next = next;
				}
			} else {
				prev = message;
			}

			message = next;
		}
	}

	public boolean hasRunnable(Runnable runnable) {
		return hasMessage(sMessageMatcherRunnable, RUNNABLE_MSG_ID, runnable);
	}

	public int cancelRunnable(Runnable runnable) {
		return removeMessage(sMessageMatcherRunnable, RUNNABLE_MSG_ID, runnable);
	}

	public int setRunnableDelay(Runnable runnable, long delay) {
		return setDelay(sMessageMatcherRunnable, RUNNABLE_MSG_ID, runnable, delay);
	}

	@Override
	public void run() {
		while (true) {
			CavanMessage message;

			synchronized (this) {
				if (mHead == null) {
					try {
						wait();
					} catch (InterruptedException e) {
						e.printStackTrace();
					}

					continue;
				}

				message = mHead;

				long timeNow = System.currentTimeMillis();

				if (message.time > timeNow) {
					try {
						wait(message.time - timeNow);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}

					continue;
				}

				mHead = message.next;
			}

			message.runnable.run();

			addFreeMessage(message);
		}
	}

	protected void handleMessage(CavanMessage message) {
		throw new InvalidParameterException("handleMessage is not implemented!");
	}

	public static void main(String[] args) {
		CavanMessageQueue queue = new CavanMessageQueue() {

			@Override
			protected void handleMessage(CavanMessage message) {
				CavanJava.dLog("message = " + message);
			}
		};


		queue.sendEmptyMessageDelayed(1, 1000);

		CavanMessage message = queue.obtainMessage(5, 1);
		queue.sendMessageDelayed(message, 5000);

		queue.sendEmptyMessageDelayed(3, 3000);
		queue.sendEmptyMessageDelayed(4, 4000);

		message = queue.obtainMessage(6, 1, 2);
		queue.sendMessageDelayed(message, 6000);

		queue.sendEmptyMessageDelayed(2, 2000);

		message = queue.obtainMessage(7, 1, 2, 3);
		queue.sendMessageDelayed(message, 7000);;

		queue.sendEmptyMessage(7);

		for (int i = 0; i < 10; i++) {
			message = queue.obtainMessage(8, 1, "A", 2, "B", 3, "C", 4, "D", 5, "E", 6, "F");
			queue.sendMessageDelayed(message, 8000);
		}

		queue.setDelay(8, 0);

		queue.sendEmptyMessageDelayed(0, 0);

		Runnable runnable1 = new Runnable() {

			@Override
			public void run() {
				CavanJava.pLog();
			}
		};

		Runnable runnable2 = new Runnable() {

			@Override
			public void run() {
				CavanJava.pLog();
			}
		};

		queue.postRunnableDelayed(runnable1, 2000);
		queue.postRunnableDelayedUniq(runnable1);

		queue.postRunnableDelayed(runnable2, 3000);
		queue.postRunnableDelayedReplace(runnable2);

		try {
			queue.join();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}
}
