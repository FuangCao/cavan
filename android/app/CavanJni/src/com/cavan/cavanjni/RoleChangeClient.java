package com.cavan.cavanjni;

import com.cavan.service.CavanCommandService;

public class RoleChangeClient extends CavanCommandService {

	@Override
	protected int doCommandMain(String command) {
		return CavanJni.doRoleChange("client", command);
	}

	@Override
	protected void doCommandStop() {
		CavanJni.kill("role_change");
	}
}
