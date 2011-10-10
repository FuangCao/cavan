all:
	$(Q)MSGFMT=$$(which msgfmt) \
	MSGMERGE=$$(which msgmerge) \
	XGETTEXT=$$(which xgettext) \
	sb2 ./configure --prefix=/usr
	$(Q)+sb2 make
	$(Q)+sb2 -m install make install-dev
	$(Q)+sb2 -m install make install-lib
