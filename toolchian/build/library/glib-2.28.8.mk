include $(MAKEFILE_DEFINES)

all:
	sed 's/\s*\(as_fn_error\)\s*(\s*)\s*$$/&\n{\n\treturn 0\n}\n\1_bak()/g' configure -i
	glib_cv_uscore=yes \
	ac_cv_func_posix_getpwuid_r=yes \
	./configure $(LIBRARY_COMMON_CONFIG) --prefix=/usr --with-sysroot=$(SYSROOT_PATH)
	make -j4
	$(call install_to_sysroot)
