all:
	MSGFMT=$$(which msgfmt) \
	MSGMERGE=$$(which msgmerge) \
	XGETTEXT=$$(which xgettext) \
	./configure $(LIBRARY_COMMON_CONFIG) --prefix=$(SYSROOT_PATH)/usr
	+make
	+make install-dev
	+make install-lib
