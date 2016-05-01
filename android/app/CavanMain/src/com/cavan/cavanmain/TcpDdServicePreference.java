package com.cavan.cavanmain;

import android.content.Context;
import android.os.RemoteException;
import android.util.AttributeSet;

public class TcpDdServicePreference extends CavanServicePreference {

	public TcpDdServicePreference(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	@Override
	protected void start(ICavanService service) throws RemoteException {
		service.startTcpDdService();
	}

	@Override
	protected boolean getState(ICavanService service) throws RemoteException {
		return service.getTcpDdServiceState();
	}

	@Override
	protected String getAction() {
		return CavanService.ACTION_TCP_DD_CHANGED;
	}

	@Override
	protected int getPort(ICavanService service) throws RemoteException {
		return service.getTcpDdServicePort();
	}

	@Override
	protected void setPort(ICavanService service, int port)
			throws RemoteException {
		service.setTcpDdServicePort(port);
	}
}
