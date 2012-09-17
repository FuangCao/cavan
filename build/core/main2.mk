ARCH ?= host
BUILD_TYPE ?= debug
BUILD_ENTRY ?= app

ROOT_PATH = $(shell pwd)
CAVAN_NAME = cavan
APP_PATH = app
LIB_PATH = lib
BUILD_PATH = build
APP_PREFIX = ${CAVAN_NAME}-
INCLUDE_PATH = $(ROOT_PATH)/include
APP_CORE_PATH = $(APP_PATH)/core
APP_OTHERS_PATH = $(APP_PATH)/others
LIB_OTHERS_PATH = $(LIB_PATH)/others
MAKEFILE_CAVAN = $(CAVAN_NAME).mk

OUT_PATH = out/$(ARCH)
OUT_LIB = $(OUT_PATH)/lib
OUT_APP = $(OUT_PATH)/app
OUT_CAVAN = $(OUT_PATH)/$(CAVAN_NAME)
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

CFLAGS +=	-Wall -Wundef -Werror -Wstrict-prototypes -Wno-trigraphs \
			-Werror-implicit-function-declaration -Wno-format-security \
			-fno-strict-aliasing -g -O2 \
			-I$(INCLUDE_PATH) -I. -DCAVAN_ARCH=$(ARCH)
ASFLAGS +=	$(CFLAGS) -D__ASM__
LDFLAGS += -lm -lpthread

DESTDIR = /usr

ifeq "$(Q)" "@"
MAKEFLAGS += --no-print-directory
endif

include $(BUILD_PATH)/core/defines2.mk

APP_SEARCH_PATHS = $(APP) $(APP_PATH)
LIB_SEARCH_PATHS = $(LIB) $(LIB_PATH)

ifeq "$(BUILD_OTHERS)" "true"
APP_SEARCH_PATHS += $(APP_OTHERS_PATH)
LIB_SEARCH_PATHS += $(LIB_OTHERS_PATH)
endif

$(foreach app,$(APP_SEARCH_PATHS),$(eval $(call build_app_action,$(app))))
$(foreach lib,$(LIB_SEARCH_PATHS),$(eval $(call build_lib_action,$(lib))))

LIB_OBJ_FILES = $(call file_path_convert,$(LIB_SRC_FILES),$(OUT_LIB)/,.o)
TARGET_LIBO = $(OUT_LIB)/$(CAVAN_NAME).o
TARGET_LIBSO = $(OUT_LIB)/lib$(CAVAN_NAME).so
TARGET_LIBA = $(OUT_LIB)/lib$(CAVAN_NAME).a

APP_OBJ_FILES = $(call file_path_convert,$(APP_SRC_FILES),$(OUT_APP)/,.o)
TARGET_BINS = $(call file_path_convert,$(APP_SRC_FILES),$(OUT_BIN)/$(APP_PREFIX))

ifeq "$(filter debug static release,$(BUILD_TYPE))" ""
$(warning this project can only build as: debug static release)
$(error unknown BUILD_TYPE = "$(BUILD_TYPE)")
endif

ifeq "$(BUILD_TYPE)" "debug"
APP_LDFLAGS += -L$(ROOT_PATH)/$(OUT_LIB) -Wl,-rpath,$(ROOT_PATH)/$(OUT_LIB) -l$(CAVAN_NAME)
endif

ifeq "$(BUILD_TYPE)" "static"
APP_DEPENDS = $(TARGET_LIBA)
APP_LDFLAGS += -static -L$(OUT_LIB) -l$(CAVAN_NAME)
endif

ifeq "$(BUILD_TYPE)" "release"
APP_DEPENDS = $(TARGET_LIBO)
APP_LDFLAGS += $(TARGET_LIBO)
endif

APP_CORE_SRC_FILES = $(wildcard $(APP_CORE_PATH)/*.c)
CAVAN_OBJ_FILES = $(call file_path_convert,$(APP_SRC_FILES) $(APP_CORE_SRC_FILES),$(OUT_CAVAN)/,.o)
CAVAN_MAP_HEADER = $(OUT_CAVAN)/cavan_map.h
CAVAN_MAP_SOURCE = $(OUT_CAVAN)/cavan_map.c
TARGET_CAVAN = $(OUT_BIN)/$(CAVAN_NAME)-main
CAVAN_CFLAGS = -I$(ROOT_PATH)/$(OUT_CAVAN)

$(foreach out-dir,$(OUT_LIB) $(OUT_APP) $(OUT_BIN) $(OUT_CAVAN),$(shell $(MKDIR) $(out-dir)))

all: $(BUILD_ENTRY)

lib: $(TARGET_LIBA) $(TARGET_LIBSO)

liba: $(TARGET_LIBA)

libso: $(TARGET_LIBSO)

ifeq "$(BUILD_TYPE)" "debug"
app: $(TARGET_LIBSO) $(APP_OBJ_FILES)
	$(Q)$(MAKE) app-base

app-base: $(TARGET_BINS)

cavan: $(TARGET_LIBSO) $(CAVAN_OBJ_FILES)
	$(Q)$(MAKE) cavan-base

cavan-base: $(TARGET_CAVAN)
else
app: $(TARGET_BINS)

cavan: $(TARGET_CAVAN)
endif

Debug debug:
	$(Q)$(MAKE) BUILD_TYPE=debug

Release release:
	$(Q)$(MAKE) BUILD_TYPE=release

Static static:
	$(Q)$(MAKE) BUILD_TYPE=static

$(OUT_BIN)/$(APP_PREFIX)%: $(OUT_APP)/%.o $(APP_DEPENDS)
	$(call link_excuteable,$@,$<)

$(TARGET_CAVAN): $(CAVAN_OBJ_FILES) $(APP_DEPENDS)
	$(call link_excuteable,$@,$(filter-out $(APP_DEPENDS),$^))

$(TARGET_LIBA): $(TARGET_LIBO)
	$(call link_static_library,$@,$^)

$(TARGET_LIBSO): $(TARGET_LIBO)
	$(call link_shared_library,$@,$^)

$(TARGET_LIBO): $(LIB_OBJ_FILES)
	$(call link_object_file,$@,$^)

$(OUT_CAVAN)/%.o: $(APP_CORE_PATH)/%.c $(CAVAN_MAP_HEADER) $(CAVAN_MAP_SOURCE)
	$(call compile_file,$@,$<,$(CAVAN_CFLAGS))

$(CAVAN_MAP_HEADER): $(APP_SRC_FILES)
	@echo "[GEN]   $@ <= $^"
	@for app in $(patsubst %.c,do_cavan_%,$(^F)); \
	do \
		echo "int $${app}(int argc, char *argv[]);"; \
	done > $@

$(CAVAN_MAP_SOURCE): $(APP_SRC_FILES)
	@echo "[GEN]   $@ <= $^"
	@for app in $(patsubst %.c,%,$(^F)); \
	do \
		echo "{\"$${app}\", do_cavan_$${app}},"; \
	done > $@

install-header: $(wildcard $(ROOT_PATH)/include/*)
	$(Q)$(CP) $^ $(DESTDIR)/include -av

install-lib: $(TARGET_LIBSO) $(TARGET_LIBA)
	$(Q)$(INSTALL) -v $^ $(DESTDIR)/lib

install-bin: $(TARGET_BINS)
	$(Q)$(INSTALL) -v $^ $(DESTDIR)/bin

install: install-header install-lib install-bin

uninstall-header:
	$(Q)$(RM) $(addprefix $(DESTDIR)/include/,$(notdir $(wildcard $(ROOT_PATH)/include/*))) -v

uninstall-lib: $(TARGET_LIBA) $(TARGET_LIBSO)
	$(Q)$(RM) $(addprefix $(DESTDIR)/lib/,$(notdir $(TARGET_LIBA) $(TARGET_LIBSO))) -v

uninstall-bin:
	$(Q)$(RM) $(addprefix $(DESTDIR)/bin/,$(notdir $(TARGET_BINS))) -v

uninstall: uninstall-header uninstall-lib uninstall-bin

config-sh config shconfig menuconfig:
	$(Q)$(ROOT_PATH)/script/cavan-config-sh

config-gtk gtkconfig:
	$(Q)$(ROOT_PATH)/script/cavan-config-gtk

clean:
	$(Q)$(RM) $(OUT_PATH) -v

clean-app:
	$(Q)$(RM) $(OUT_APP) -v

clean-bin:
	$(Q)$(RM) $(OUT_BIN) -v

clean-lib:
	$(Q)$(RM) $(OUT_LIB) -v

clean-cavan:
	$(Q)$(RM) $(OUT_CAVAN) -v

.PRECIOUS: $(APP_OBJ_FILES) $(LIB_OBJ_FILES) $(CAVAN_OBJ_FILES)
.PHONY: uninstall uninstall-header uninstall-lib uninstall-bin
