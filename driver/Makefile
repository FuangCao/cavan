ROOT_DIR = $(shell pwd -P)
KERNEL_VERSION = $(shell uname -r)
KERNEL_PATH ?= /lib/modules/$(KERNEL_VERSION)/build
ARCH ?= arm

ifeq "$(ARCH)" "arm"
CROSS_COMPILE ?= arm-cavan-linux-gnueabi-
endif

Q ?= @

$(info KERNEL_PATH = $(KERNEL_PATH))
$(info ARCH = $(ARCH))
$(info CROSS_COMPILE = $(CROSS_COMPILE))

obj-m += swan_vk.o swan_vn.o

KERNEL_MAKE = +make -C $(KERNEL_PATH) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)
RM = rm -rfv

all:
	$(Q)$(KERNEL_MAKE) M=$(PWD)

clean:
	$(Q)$(RM) *.o *.mod.c *.ko .*.cmd
	$(Q)$(RM) modules.order Module.symvers .tmp_versions
