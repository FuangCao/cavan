all:
	PKG_CONFIG_LIBDIR=$(UTILS_PATH)/usr/lib/pkgconfig \
	./configure \
	--prefix=$(UTILS_PATH)/usr \
	--target-list=$(CAVAN_TARGET_ARCH)-softmmu,$(CAVAN_TARGET_ARCH)-linux-user \
	--cc='$(CAVAN_BUILD_PLAT)-gcc -I$(UTILS_PATH)/usr/include -L$(UTILS_PATH)/usr/lib' \
	--disable-kvm
	make -j4
	make install
