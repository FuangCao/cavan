all:
	sed 's,^\s*SHELL\s*=.*$$,SHELL = /bin/bash,g' include/buildmacros -i
	sb2 ./configure --prefix=/usr
	+sb2 make
	+sb2 -m install make install-dev
	+sb2 -m install make install-lib
