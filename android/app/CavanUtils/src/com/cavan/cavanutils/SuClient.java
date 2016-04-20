package com.cavan.cavanutils;

import android.net.LocalSocketAddress;

public class SuClient extends TcpExecClient {

	public SuClient() {
		super(new LocalSocketAddress("/dev/cavan/network/socket", LocalSocketAddress.Namespace.FILESYSTEM));
	}
}