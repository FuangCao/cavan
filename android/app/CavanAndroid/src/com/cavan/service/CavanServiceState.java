package com.cavan.service;

public interface CavanServiceState {

	public static final int STOPPED = 0;
	public static final int PREPARE = 1;
	public static final int RUNNING = 2;
	public static final int WAITING = 3;
	public static final int STARTING = 4;
	public static final int CONNECTING = 5;
	public static final int CONNECTED = 6;
	public static final int DISCONNECTING = 7;
	public static final int DISCONNECTED = 8;
}