ARCH ?= host
ROOT_PATH = $(shell pwd)
CAVAN_NAME = cavan
BUILD_PATH = $(ROOT_PATH)/build
BUILD_CORE_PATH = $(BUILD_PATH)/core
APP_CORE_PATH = $(ROOT_PATH)/app/core
INCLUDE_PATH = $(ROOT_PATH)/include
SUB_DIRS = lib app

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
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar
STRIP = $(CROSS_COMPILE)strip
RM = rm -rf
MAKE = +make
MKDIR = mkdir -p
INSTALL = install -c
CP = cp

CAVAN_PLAT = $(shell $(CC) -dumpmachine)

CFLAGS +=	-Wall -Wundef -Werror -Wstrict-prototypes -Wsign-compare -Werror-implicit-function-declaration \
			-Wno-trigraphs -Wno-format-security \
			-Wpointer-arith -fno-strict-aliasing -g -O2 -I$(INCLUDE_PATH) -I. \
			-DCAVAN_ARCH=\"$(ARCH)\" -DCAVAN_PLAT=\"$(CAVAN_PLAT)\"

LDFLAGS += -lm -lrt -lpthread

ifeq ($(BUILD_TYPE),debug)
CFLAGS += -DCAVAN_DEBUG
endif

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

clean:
	@rm $(OUT_PATH) -rf

$(foreach sub,$(SUB_DIRS),$(eval $(call build_sub_module,$(sub))))

all-modules: $(MODULES)

.PRECIOUS: $(APP_OBJ_FILES) $(LIB_OBJ_FILES) $(CAVAN_OBJ_FILES)
.PHONY: uninstall uninstall-header uninstall-lib uninstall-bin
