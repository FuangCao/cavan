$(info CAVAN_ROOT = $(CAVAN_ROOT))

all:
	$(Q)+make -C $(CAVAN_ROOT) $(MAKE_TYPE)=$(shell pwd)

clean:
	$(Q)+make -C $(CAVAN_ROOT) clean

distclean:
	$(Q)+make -C $(CAVAN_ROOT) distclean
