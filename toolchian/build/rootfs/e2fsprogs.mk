include $(MAKEFILE_DEFINES)

all:
	sb2 ./configure --prefix=/
	sb2 while ! make; \
	do \
		chmod 555 lib/ext2fs/ext2_err.[ch] -v || continue; \
		chmod 555 lib/ss/ss_err.[ch] -v || continue; \
		chmod 555 e2fsck/prof_err.[ch] -v || continue; \
	done
	$(call install_to_rootfs)
