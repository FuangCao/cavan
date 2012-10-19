KERNEL_VERSION = $(shell uname -r)
KERNEL_PATH ?= /lib/modules/$(KERNEL_VERSION)/build
ARCH ?= arm
CROSS_COMPILE ?= arm-cavan-linux-gnueabi-

$(info KERNEL_PATH = $(KERNEL_PATH))
$(info ARCH = $(ARCH))
$(info CROSS_COMPILE = $(CROSS_COMPILE))

obj-m += swan_vk.o swan_vn.o

MAKE = @make
RM = @rm -rfv

export ARCH CROSS_COMPILE

all:
	$(MAKE) -C $(KERNEL_PATH) M=$(PWD)

clean:
	$(RM) *.o *.mod.c *.ko .*.cmd
	$(RM) modules.order Module.symvers .tmp_versions
