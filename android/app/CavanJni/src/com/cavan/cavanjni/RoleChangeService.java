package com.cavan.cavanjni;


public class RoleChangeService extends CavanNativeCommandService {

	public RoleChangeService() {
		super(new CavanCommandRoleChange());
	}

	@Override
	protected String[] buildCommandArgs(String command) {
		return new String[] { "service", command };
	}
}
