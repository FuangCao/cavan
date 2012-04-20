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

CFLAGS +=	-Wall -Wundef -Werror -Wstrict-prototypes -Wno-trigraphs \
			-Werror-implicit-function-declaration -Wno-format-security \
			-fno-strict-aliasing -g -O2 \
			-I$(INCLUDE_PATH) -I. -DCAVAN_ARCH=$(ARCH)
ASFLAGS +=	$(CFLAGS) -D__ASM__
LDFLAGS += -lm -lpthread

ifeq ("$(Q)","@")
MAKEFLAGS += --no-print-directory
endif

include $(BUILD_PATH)/core/defines2.mk

$(foreach app,$(APP) $(APP_PATH),$(eval $(call build_app_action,$(app))))
$(foreach lib,$(LIB) $(LIB_PATH),$(eval $(call build_lib_action,$(lib))))

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

CAVAN_SRC_FILES = $(call file_path_convert,$(APP_SRC_FILES),$(OUT_CAVAN)/,.c)
APP_CORE_SRC_FILES = $(wildcard $(APP_CORE_PATH)/*.c)
APP_CORE_OBJ_FILES = $(call file_path_convert,$(APP_CORE_SRC_FILES),$(OUT_CAVAN)/,.o)
CAVAN_OBJ_FILES = $(CAVAN_SRC_FILES:%.c=%.o) $(APP_CORE_OBJ_FILES)
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

$(OUT_CAVAN)/%.o: $(OUT_CAVAN)/%.c
	$(call compile_file,$@,$<,$(CAVAN_CFLAGS))

$(CAVAN_MAP_HEADER): $(CAVAN_SRC_FILES)
	@echo "[GEN]\t$@ <= $^"
	@for app in $(patsubst %.c,do_cavan_%,$(^F)); \
	do \
		echo "int $${app}(int argc, char *argv[]);"; \
	done > $@

$(CAVAN_MAP_SOURCE): $(CAVAN_SRC_FILES)
	@echo "[GEN]\t$@ <= $^"
	@for app in $(patsubst %.c,%,$(^F)); \
	do \
		echo "{\"$${app}\", do_cavan_$${app}},"; \
	done > $@

clean:
	$(Q)$(RM) $(OUT_PATH)

.PRECIOUS: $(APP_OBJ_FILES) $(LIB_OBJ_FILES) $(CAVAN_SRC_FILES) $(CAVAN_OBJ_FILES)
