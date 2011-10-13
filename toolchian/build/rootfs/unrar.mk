all:
	$(Q)+sb2 make -f makefile.unix
	$(Q)+sb2 -m install cp unrar /usr/bin -av
