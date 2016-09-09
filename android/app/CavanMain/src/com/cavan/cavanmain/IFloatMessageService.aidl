package com.cavan.cavanmain;

interface IFloatMessageService {
	boolean getTimerState();
	boolean setTimerEnable(boolean enable);
	int addMessage(CharSequence message, CharSequence code, int delay);
	boolean hasMessage(CharSequence message);
	void removeMessage(CharSequence message);
	List<String> getMessages();
	List<String> getCodes();
	int getCodeDelay(String code);
}