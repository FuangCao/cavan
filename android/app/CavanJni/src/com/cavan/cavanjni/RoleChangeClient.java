package com.cavan.cavanjni;


public class RoleChangeClient extends CavanNativeCommandService {

	public RoleChangeClient() {
		super(new CavanCommandRoleChange());
	}

	@Override
	protected String[] buildCommandArgs(String command) {
		return new String[] { "client", command };
	}
}
