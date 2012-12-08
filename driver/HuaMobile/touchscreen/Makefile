KERNEL_PATH = ../../../../../out/target/product/hsdroid/obj/KERNEL.cp8056
ARCH ?= arm
CROSS_COMPILE ?= arm-cavan-linux-gnueabi-

obj-m += hua_ts_core.o hua_ft5216.o hua_cy8c242.o

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNEL_PATH) M=$(PWD)

clean:
	@rm -rfv *.o *.ko *.mod.* .*.cmd
	@rm -rfv .tmp_versions modules.order Module.symvers
