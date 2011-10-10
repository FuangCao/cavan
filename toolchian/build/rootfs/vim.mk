include $(MAKEFILE_DEFINES)

all:
	sed 's/\s*\(as_fn_error\)\s*(\s*)\s*$$/&\n{\n\treturn 0\n}\n\1_bak()/g' src/auto/configure -i
	ac_cv_sizeof_int=4 \
	vim_cv_getcwd_broken=no \
	vim_cv_memmove_handles_overlap=yes \
	vim_cv_stat_ignores_slash=yes \
	vim_cv_tgetent=zero \
	vim_cv_terminfo=yes \
	vim_cv_toupper_broken=no \
	vim_cv_tty_group=world \
	STRIP=$(CAVAN_TARGET_PLAT)-strip \
	./configure $(ROOTFS_COMMON_CONFIG) --prefix=/usr  --with-tlib=ncurses
	+make
	$(call install_to_rootfs)
