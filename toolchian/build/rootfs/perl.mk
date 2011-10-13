all:
	$(Q)sb2 ./Configure -de -Dcc=gcc -Dprefix=/usr
	$(Q)+sb2 make
	$(Q)+sb2 make install
