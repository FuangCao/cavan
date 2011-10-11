all:
	$(Q)+sb2 make CC=gcc
	$(Q)+sb2 -m install make PREFIX=/usr install
