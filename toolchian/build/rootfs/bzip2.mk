all:
	$(Q)+sb2 make CC="gcc\ -fPIC"
	$(Q)+sb2 -m install make PREFIX=/usr install
