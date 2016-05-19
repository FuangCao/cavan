package com.cavan.cavanutils;

import com.cavan.cavanutils.ScanResult;

interface IDiscoveryService {
	boolean scan(int port);
	List<com.cavan.cavanutils.ScanResult> getScanResult();
}