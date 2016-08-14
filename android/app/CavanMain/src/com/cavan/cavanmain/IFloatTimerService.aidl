package com.cavan.cavanmain;

interface IFloatTimerService {
	boolean getTimerState();
	boolean setTimerEnable(boolean enable);
	int addMessage(CharSequence message);
	boolean hasMessage(CharSequence message);
	void removeMessage(CharSequence message);
}