all:
	$(Q)CFLAGS="-fPIC" sb2 ./configure --prefix=/usr
	$(Q)sb2 make
	$(Q)sb2 -m install make install
