KERNEL_PATH = $(HOME)/kernel
ARCH ?= arm
CROSS_COMPILE ?= arm-cavan-linux-gnueabi-
EXTRA_CFLAGS = -Wall -Werror -I$(PWD)/include

hua_input_core-objs = core/hua_input_core.o core/hua_sensor_core.o core/hua_ts_core.o

obj-m += sensor/hua_adxl34x.o sensor/hua_lis3dh.o sensor/hua_mxc6225.o sensor/hua_kionix.o sensor/hua_mma845x.o
obj-m += touchscreen/hua_cy8c242.o touchscreen/hua_ft5216.o
obj-m += misc/swan_vk.o misc/swan_vn.o
obj-m += hua_input_core.o

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNEL_PATH) M=$(PWD)

clean:
	@for dir in . core touchscreen sensor misc; \
	do \
		cd $(PWD)/$${dir} && \
		{ \
			rm -rfv *.o *.ko *.mod.* .*.cmd; \
			rm -rfv .tmp_versions modules.order Module.symvers; \
		}; \
	done


