all:
	sed 's,^\s*SHELL\s*=.*$$,SHELL = /bin/bash,g' include/buildmacros -i
	./configure $(LIBRARY_COMMON_CONFIG) --prefix=$(SYSROOT_PATH)/usr
	+make
	+make install-dev
	+make install-lib
