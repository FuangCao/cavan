ROOT_PATH = $(shell pwd)
BUILD_PATH = $(ROOT_PATH)/build
MAKEFILE_PATH = $(BUILD_PATH)/main.mk

obj-y := core/ misc/ touchscreen/ sensor/ flash/

all:
	+make ARCH=arm -f $(MAKEFILE_PATH)

host:
	+make ARCH=$(shell uname -m) -f $(MAKEFILE_PATH)

clean:
	@for dir in . core touchscreen sensor misc; \
	do \
		cd $(PWD)/$${dir} && \
		{ \
			rm -rfv *.o *.ko *.mod.* .*.cmd; \
			rm -rfv .tmp_versions modules.order Module.symvers; \
		}; \
	done
