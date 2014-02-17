ARCH ?= arm

ifeq "$(ARCH)" "arm"
KERNEL_PATH = $(OUT)/obj/KERNEL_OBJ
[ -d "$(KERNEL_PATH)" ] || KERNEL_PATH = "$(HOME)/kernel"
CROSS_COMPILE = arm-cavan-linux-gnueabi-
else
KERNEL_PATH = /lib/modules/$(shell uname -r)/build
CROSS_COMPILE =
endif

CFLAGS = -Wall -Werror -I$(PWD)/include

all:
	+make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNEL_PATH) M=$(PWD) EXTRA_CFLAGS="$(CFLAGS)"
