all:
	MSGFMT=$$(which msgfmt) \
	MSGMERGE=$$(which msgmerge) \
	XGETTEXT=$$(which xgettext) \
	sb2 ./configure --prefix=/usr
	+sb2 make
	+sb2 -m install make install-dev
	+sb2 -m install make install-lib
