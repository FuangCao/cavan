ARCH ?= host
ROOT_PATH = $(shell pwd)
CAVAN_NAME = cavan
BUILD_PATH = $(ROOT_PATH)/build
BUILD_CORE_PATH = $(BUILD_PATH)/core
APP_CORE_PATH = $(ROOT_PATH)/app/core
INCLUDE_PATH = $(ROOT_PATH)/include
SUB_DIRS = lib cpp app test

APP_PREFIX = ${CAVAN_NAME}-
MAKEFILE_CAVAN = $(CAVAN_NAME).mk
BUILD_LIBA = $(BUILD_CORE_PATH)/build_liba.mk
BUILD_LIBSO = $(BUILD_CORE_PATH)/build_libso.mk
BUILD_EXECUTE = $(BUILD_CORE_PATH)/build_execute.mk
BUILD_EXEC_PKG = $(BUILD_CORE_PATH)/build_exec_pkg.mk

OUT_PATH = out/$(ARCH)
OUT_LIB = $(OUT_PATH)/lib
OUT_OBJ = $(OUT_PATH)/obj
OUT_BIN = $(OUT_PATH)/$(BUILD_TYPE)

CC = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar
STRIP = $(CROSS_COMPILE)strip
RM = rm -rf
MAKE = +make
MKDIR = mkdir -p
INSTALL = install -c
CP = cp

CAVAN_PLAT = $(shell $(CC) -dumpmachine)

CFLAGS +=	-Wall -Wundef -Werror -Wsign-compare -Winit-self -Wpointer-arith -Wa,--noexecstack -Wstrict-aliasing=2 \
			-fno-strict-aliasing -fno-exceptions -fno-inline-functions-called-once -fno-short-enums \
			-ffunction-sections -funwind-tables -fstack-protector -finline-functions -Wmissing-declarations \
			-fgcse-after-reload -frerun-cse-after-loop -frename-registers -fomit-frame-pointer -finline-limit=64 \
			-g -Os -I$(INCLUDE_PATH) -DCAVAN_ARCH=\"$(ARCH)\" -DCAVAN_PLAT=\"$(CAVAN_PLAT)\"

ifeq ($(BUILD_TYPE),debug)
CFLAGS += -DCAVAN_DEBUG
endif

CPPFLAGS := $(CPPFLAGS) $(CFLAGS)
CFLAGS += -Werror-implicit-function-declaration -Wstrict-prototypes
LDFLAGS += -lm -lrt -lpthread

ifeq ($(BUILD_TYPE),static)
LDFLAGS += -static
endif

ASFLAGS += $(CFLAGS) -D__ASM__
LDFLAGS += -L$(OUT_LIB) -Wl,-rpath,$(ROOT_PATH)/$(OUT_LIB)

DESTDIR = /usr

ifeq ($(Q),@)
MAKEFLAGS += --no-print-directory
endif

include $(BUILD_CORE_PATH)/defines3.mk

$(foreach path,$(OUT_LIB) $(OUT_BIN),$(shell [ -d $(path) ] || $(MKDIR) $(path)))

all: all-modules

clean distclean:
	@rm $(OUT_PATH) -rf

$(foreach sub,$(SUB_DIRS),$(eval $(call build_sub_module,$(sub))))

all-modules: $(MODULES)

.PRECIOUS: $(APP_OBJ_FILES) $(LIB_OBJ_FILES) $(CAVAN_OBJ_FILES)
.PHONY: uninstall uninstall-header uninstall-lib uninstall-bin
