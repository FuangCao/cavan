all:
	MSGFMT=$$(which msgfmt) \
	MSGMERGE=$$(which msgmerge) \
	XGETTEXT=$$(which xgettext) \
	sb2 ./configure --prefix=$(EMULATOR_PATH)/usr
	+sb2 make
	+sb2 make install-dev
	+sb2 make install-lib
