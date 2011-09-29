COMMON_OPTION = --prefix=/usr --build=$(CAVAN_BUILD_PLAT) --host=$(CAVAN_BUILD_PLAT) --target=$(CAVAN_TARGET_PLAT)

ARC ?= .

all:
	$(Q)$(SRC)/configure $(OPT) $(COMMON_OPTION)
	$(Q)+make
	$(Q)+make DESTDIR=$(SYSROOT_PATH) install
