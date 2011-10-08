all:
	PKG_CONFIG_LIBDIR=$(UTILS_PATH)/usr/lib/pkgconfig \
	./configure --prefix=$(UTILS_PATH)/usr --disable-tests
	make -j4
	make install
