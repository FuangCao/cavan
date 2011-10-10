all:
	$(Q)+sb2 ./configure --prefix=/
	$(Q)+sb2 while ! make; \
	$(Q)do \
		chmod 555 lib/ext2fs/ext2_err.[ch] -v || continue; \
		chmod 555 lib/ss/ss_err.[ch] -v || continue; \
		chmod 555 e2fsck/prof_err.[ch] -v || continue; \
	done
	$(Q)+sb2 -m install make install
