all:
	$(Q)+sb2 make CC=gcc -C src
	$(Q)+sb2 -m install make -C src install
