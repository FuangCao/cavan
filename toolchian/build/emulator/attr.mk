all:
	sed 's,^\s*SHELL\s*=.*$$,SHELL = /bin/bash,g' include/buildmacros -i
	sb2 ./configure --prefix=$(EMULATOR_PATH)/usr
	+sb2 make
	+sb2 make install-dev
	+sb2 make install-lib
