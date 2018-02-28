package com.cavan.cavanjni;

import com.cavan.service.CavanCommandService;

public class CavanNativeCommandService extends CavanCommandService {

	private CavanNativeCommand mCommand;

	public CavanNativeCommandService(CavanNativeCommand command) {
		mCommand = command;
	}

	protected String[] buildCommandArgs(String command) {
		return new String[] { command };
	}

	@Override
	protected int doCommandMain(String command) {
		return mCommand.run(buildCommandArgs(command));
	}

	@Override
	protected void doCommandStop() {
		mCommand.kill();
	}
}
