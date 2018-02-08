package com.cavan.cavanjni;

public abstract class CavanNativeCommand {

	private int mPid;

	public synchronized int getPid() {
		return mPid;
	}

	public boolean kill() {
		int pid = getPid();

		if (pid != 0 && CavanJni.kill(pid) < 0) {
			return false;
		}

		return true;
	}

	public boolean waitpid() {
		int pid = getPid();

		if (pid != 0 && CavanJni.waitpid(pid) < 0) {
			return false;
		}

		return true;
	}

	public int run(String... args) {
		try {
			int pid = main(true, args);
			if (pid < 0) {
				return pid;
			}

			synchronized (this) {
				mPid = pid;
			}

			return CavanJni.waitpid(pid);
		} catch (Exception e) {
			e.printStackTrace();
			return -1;
		} finally {
			synchronized (this) {
				mPid = 0;
			}
		}
	}

	public abstract int main(boolean async, String... args);
}
