KERNEL_PATH = ../../../../../out/target/product/hsdroid/obj/KERNEL.cp8056
ARCH ?= arm
CROSS_COMPILE ?= arm-cavan-linux-gnueabi-

EXTRA_CFLAGS = -Wall -Werror

obj-m += hua_sensor_core.o hua_adxl34x.o hua_lis3dh.o

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNEL_PATH) M=$(PWD)

clean:
	@rm -rfv *.o *.ko *.mod.* .*.cmd
	@rm -rfv .tmp_versions modules.order Module.symvers
