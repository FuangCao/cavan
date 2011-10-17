all:
	$(Q)+sb2 make CC="gcc\ -fPIC" -f makefiles/makefile.linux
	$(Q)+sb2 -m install make prefix=/usr -f makefiles/makefile.linux install
