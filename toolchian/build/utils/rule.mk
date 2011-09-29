COMMON_OPTION = --prefix=/usr

SRC ?= .

all:
	$(Q)$(SRC)/configure $(OPT) $(COMMON_OPTION)
	$(Q)+make
	$(Q)+make DESTDIR=$(UTILS_PATH) install
