package com.cavan.cavanmain;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;

import com.cavan.cavanutils.CavanBaseService;
import com.cavan.cavanutils.CavanUtils;

public class CavanService extends Service {

	public static final String ACTION_FTP_CHANGED = "cavan.intent.action.FTP_CHANGED";
	public static final String ACTION_TCP_DD_CHANGED = "cavan.intent.action.TCP_DD_CHANGED";
	public static final String ACTION_WEB_PROXY_CHANGED = "cavan.intent.action.WEB_PROXY_CHANGED";

	private FtpService mFtpService = new FtpService(2121);
	private TcpDdService mTcpDdService = new TcpDdService(8888);
	private WebProxyService mWebProxyService = new WebProxyService(9090);

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

		@Override
		public int getTcpDdServicePort() throws RemoteException {
			return mTcpDdService.getPort();
		}

		@Override
		public void setTcpDdServicePort(int port) throws RemoteException {
			mTcpDdService.setPort(port);
		}

		@Override
		public int getWebProxyServicePort() throws RemoteException {
			return mWebProxyService.getPort();
		}

		@Override
		public void setWebProxyServicePort(int port) throws RemoteException {
			mWebProxyService.setPort(port);
		}

		@Override
		public int getFtpServicePort() throws RemoteException {
			return mFtpService.getPort();
		}

		@Override
		public void setFtpServicePort(int port) throws RemoteException {
			mFtpService.setPort(port);
		}
	}

	class TcpDdService extends CavanBaseService {

		public TcpDdService(int port) {
			super(CavanService.this, TcpDdService.class.getName(), ACTION_TCP_DD_CHANGED, port);
		}

		@Override
		public void runDeamon(int port) {
			CavanUtils.doTcpDdServer("-p", Integer.toString(port), "-s", "0", "--discovery");
		}
	}

	class FtpService extends CavanBaseService {

		public FtpService(int port) {
			super(CavanService.this, FtpService.class.getName(), ACTION_FTP_CHANGED, port);
		}

		@Override
		public void runDeamon(int port) {
			CavanUtils.doFtpServer("-p", Integer.toString(port));
		}
	}

	class WebProxyService extends CavanBaseService {

		public WebProxyService(int port) {
			super(CavanService.this, WebProxyService.class.getName(), ACTION_WEB_PROXY_CHANGED, port);
		}

		@Override
		public void runDeamon(int port) {
			CavanUtils.doWebProxy("-p", Integer.toString(port));
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
