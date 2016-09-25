package com.cavan.cavanmain;

import com.cavan.cavanmain.RedPacketCode;

interface IFloatMessageService {
	boolean getTimerState();
	boolean setTimerEnable(boolean enable);
	int addMessage(CharSequence message, in RedPacketCode code, boolean test);
	boolean hasMessage(CharSequence message);
	void removeMessage(CharSequence message);
	List<String> getMessages();
	int getMessageCount();

	List<RedPacketCode> getCodes();
	int getCodeCount();

	boolean sendRedPacketCode(String code);
	boolean sendUdpCommand(String command);
	boolean sendTcpCommand(String command);

	void updateTcpService();
	void updateTcpBridge();
}