package com.cavan.cavanmain;

import com.cavan.cavanmain.RedPacketCode;

interface IFloatMessageService {
	boolean getTimerState();
	boolean setTimerEnable(boolean enable);
	int addMessage(CharSequence message, in RedPacketCode code);
	boolean hasMessage(CharSequence message);
	void removeMessage(CharSequence message);
	List<String> getMessages();
	int getMessageCount();

	List<RedPacketCode> getCodes();
	int getCodeCount();
	boolean sendSharedCode(String code);
}