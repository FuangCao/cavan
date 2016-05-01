package com.cavan.cavanmain;

import android.content.Context;
import android.os.RemoteException;
import android.util.AttributeSet;

public class WebProxyServicePreference extends CavanServicePreference {

	public WebProxyServicePreference(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	@Override
	public void start(ICavanService service) throws RemoteException {
		service.startWebProxyService();
	}

	@Override
	public boolean getState(ICavanService service) throws RemoteException {
		return service.getWebProxyServiceState();
	}

	@Override
	protected String getAction() {
		return CavanService.ACTION_WEB_PROXY_CHANGED;
	}
}
