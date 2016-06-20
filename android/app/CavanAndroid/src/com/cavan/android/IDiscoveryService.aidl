package com.cavan.android;

import com.cavan.android.ScanResult;

interface IDiscoveryService {
	boolean scan(int port);
	List<com.cavan.android.ScanResult> getScanResult();
}