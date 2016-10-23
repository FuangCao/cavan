package com.cavan.cavanmain;

interface IFloatMessageService {
	boolean getTimerState();
	boolean setTimerEnable(boolean enable);
	int addMessage(CharSequence message, String code);
	boolean hasMessage(CharSequence message);
	void removeMessage(CharSequence message);
	List<String> getMessages();
	int getMessageCount();

	List<String> getCodes();
	int getCodeCount();

	boolean sendRedPacketCode(String code);
	boolean sendUdpCommand(String command);
	boolean sendTcpCommand(String command);

	void updateTcpService();
	void updateTcpBridge();
}