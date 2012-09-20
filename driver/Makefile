KERNEL_VERSION = $(shell uname -r)
KERNEL_PATH ?= /lib/modules/$(KERNEL_VERSION)/build

$(info KERNEL_PATH = $(KERNEL_PATH))

obj-m += swan_vk.o swan_vn.o

MAKE = @make
RM = @rm -rfv

all:
	$(MAKE) -C $(KERNEL_PATH) M=$(PWD)

clean:
	$(RM) *.o *.mod.c *.ko .*.cmd
	$(RM) modules.order Module.symvers .tmp_versions
