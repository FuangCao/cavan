libc_cv_forced_unwind = yes
libc_cv_c_cleanup = yes
libc_cv_gnu99_inline = yes
HOST_CC = gcc
CC = $(CAVAN_TARGET_PLAT)-gcc
AR = $(CAVAN_TARGET_PLAT)-ar
RANLIB = $(CAVAN_TARGET_PLAT)-ranlib

GLIBC_OPTION =	--prefix=/usr \
				--build=$(CAVAN_BUILD_PLAT) \
				--host=$(CAVAN_TARGET_PLAT) \
				--disable-profile \
				--enable-add-ons \
				--with-tls \
				--enable-kernel=$(LOWEST_KERNEL_VERSION) \
				--with-__thread \
				--with-binutils=$(SYSROOT_PATH)/usr/bin \
				--with-headers=$(SYSROOT_PATH)/usr/include \

export libc_cv_forced_unwind libc_cv_c_cleanup libc_cv_gnu99_inline
export HOST_CC CC AR RANLIB

all:
	$(Q)echo "install_root=$(SYSROOT_PATH)" > configparms
	$(Q)$(SRC_GLIBC)/configure $(GLIBC_OPTION)
	$(Q)+make
	$(Q)+make install
