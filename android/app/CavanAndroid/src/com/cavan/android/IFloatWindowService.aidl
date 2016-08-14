package com.cavan.android;

interface IFloatWindowService {
	int addText(CharSequence text, int index);
	void removeText(CharSequence text);
	void removeTextAt(int index);
	void removeTextById(int id);
	void removeAll();
}