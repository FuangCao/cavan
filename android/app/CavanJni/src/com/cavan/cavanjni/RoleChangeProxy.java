package com.cavan.cavanjni;


public class RoleChangeProxy extends CavanNativeCommandService {

	public RoleChangeProxy() {
		super(new CavanCommandRoleChange());
	}

	@Override
	protected String[] buildCommandArgs(String command) {
		String[] args = command.split("\\s+");
		String[] newArgs = new String[args.length + 1] ;

		newArgs[0] = "proxy";

		for (int i = 0; i < args.length; i++) {
			newArgs[i + 1] = args[i];
		}

		return newArgs;
	}
}
