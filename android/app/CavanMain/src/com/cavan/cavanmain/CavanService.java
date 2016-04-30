package com.cavan.cavanmain;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;

import com.cavan.cavanutils.CavanBaseService;
import com.cavan.cavanutils.CavanUtils;

public class CavanService extends Service {

	public static final String ACTION_TCP_DD_CHANGED = "cavan.intent.action.TCP_DD_CHANGED";
	public static final String ACTION_FTP_CHANGED = "cavan.intent.action.FTP_CHANGED";
	public static final String ACTION_WEB_PROXY_CHANGED = "cavan.intent.action.WEB_PROXY_CHANGED";

	private FtpService mFtpService = new FtpService();
	private TcpDdService mTcpDdService = new TcpDdService();
	private WebProxyService mWebProxyService = new WebProxyService();

	class MyBinder extends ICavanService.Stub {

		@Override
		public void startFtpService() throws RemoteException {
			mFtpService.start();
		}

		@Override
		public boolean getFtpServiceState() throws RemoteException {
			return mFtpService.getState();
		}

		@Override
		public void startTcpDdService() throws RemoteException {
			mTcpDdService.start();
		}

		@Override
		public boolean getTcpDdServiceState() throws RemoteException {
			return mTcpDdService.getState();
		}

		@Override
		public void startWebProxyService() throws RemoteException {
			mWebProxyService.start();
		}

		@Override
		public boolean getWebProxyServiceState() throws RemoteException {
			return mWebProxyService.getState();
		}
	}

	class TcpDdService extends CavanBaseService {

		public TcpDdService() {
			super(CavanService.this, TcpDdService.class.getName(), ACTION_TCP_DD_CHANGED);
		}

		@Override
		public void runDeamon() {
			CavanUtils.doTcpDdServer("-p", "9999", "-s", "0", "--discovery");
		}
	}

	class FtpService extends CavanBaseService {

		public FtpService() {
			super(CavanService.this, FtpService.class.getName(), ACTION_FTP_CHANGED);
		}

		@Override
		public void runDeamon() {
			CavanUtils.doFtpServer("-p", "2121");
		}
	}

	class WebProxyService extends CavanBaseService {

		public WebProxyService() {
			super(CavanService.this, WebProxyService.class.getName(), ACTION_WEB_PROXY_CHANGED);
		}

		@Override
		public void runDeamon() {
			CavanUtils.doWebProxy("-p", "9090");
		}
	}

	@Override
	public IBinder onBind(Intent arg0) {
		return new MyBinder();
	}

	@Override
	public void onCreate() {
		CavanUtils.logE(getClass().getName() + ": onCreate");
		super.onCreate();
	}

	@Override
	public void onDestroy() {
		CavanUtils.logE(getClass().getName() + ": onDestroy");
		super.onDestroy();
	}
}
