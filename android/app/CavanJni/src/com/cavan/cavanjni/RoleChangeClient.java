package com.cavan.cavanjni;

import com.cavan.service.CavanCommandService;

public class RoleChangeClient extends CavanCommandService {

	private int mPid;

	@Override
	protected int doCommandMain(String command) {
		int pid = CavanJni.doRoleChange(true, "client", command);
		if (pid < 0) {
			return pid;
		}

		mPid = pid;
		CavanJni.waitpid(pid);
		mPid = 0;

		return 0;
	}

	@Override
	protected void doCommandStop() {
		if (mPid != 0) {
			CavanJni.kill(mPid);
		}
	}
}
