all:
	$(Q)sed 's,^\s*SHELL\s*=.*$$,SHELL = /bin/bash,g' include/buildmacros -i
	$(Q)sb2 ./configure --prefix=/usr
	$(Q)+sb2 make
	$(Q)+sb2 -m install make install-dev
	$(Q)+sb2 -m install make install-lib
