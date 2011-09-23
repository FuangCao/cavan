$(info CAVAN_ROOT = $(CAVAN_ROOT))

all:
	@make -C $(CAVAN_ROOT) $(MAKE_TYPE)=$(shell pwd)

clean:
	@make -C $(CAVAN_ROOT) clean

distclean:
	@make -C $(CAVAN_ROOT) distclean
