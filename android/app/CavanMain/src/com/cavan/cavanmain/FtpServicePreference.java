package com.cavan.cavanmain;

import android.content.Context;
import android.os.RemoteException;
import android.util.AttributeSet;

public class FtpServicePreference extends CavanServicePreference {

	public FtpServicePreference(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	@Override
	protected void start(ICavanService service) throws RemoteException {
		service.startFtpService();
	}

	@Override
	protected boolean getState(ICavanService service) throws RemoteException {
		return service.getFtpServiceState();
	}

	@Override
	protected String getAction() {
		return CavanService.ACTION_FTP_CHANGED;
	}

	@Override
	protected int getPort(ICavanService service) throws RemoteException {
		return	service.getFtpServicePort();
	}

	@Override
	protected void setPort(ICavanService service, int port)
			throws RemoteException {
		service.setFtpServicePort(port);
	}
}
