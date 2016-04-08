package com.cavan.remotecontrol;

import com.cavan.remotecontrol.ScanResult;

interface IDiscoveryService {
	boolean startDiscovery(int port);
	List<com.cavan.remotecontrol.ScanResult> getScanResult();
}