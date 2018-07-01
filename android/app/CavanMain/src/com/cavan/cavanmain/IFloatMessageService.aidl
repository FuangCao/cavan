package com.cavan.cavanmain;

interface IFloatMessageService {
	boolean getTimerState();
	boolean setTimerEnable(boolean enable);
	int addMessage(CharSequence message, String code, int level);
	boolean hasMessage(CharSequence message);
	void removeMessage(CharSequence message);
	void removeMessageAll();
	List<String> getMessages();
	int getMessageCount();

	List<String> getCodes();
	int getCodeCount();
	int getCodePending();

	boolean sendRedPacketCode(String code);
	boolean sendUdpCommand(String command);
	boolean sendTcpCommand(String command);

	void updateTcpService();
	void updateTcpBridge();

	boolean isSuspendDisabled();
	void setSuspendDisable(boolean disable);
}