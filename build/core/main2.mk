ROOT_PATH = $(shell pwd)
CAVAN_NAME = cavan
APP_PATH = app
LIB_PATH = lib
INCLUDE_PATH = $(ROOT_PATH)/include
APP_CORE_PATH = $(APP_PATH)/core

OUT_PATH = out/$(ARCH)/$(BUILD_TYPE)
OUT_LIB = $(OUT_PATH)/lib
OUT_APP = $(OUT_PATH)/app
OUT_BIN = $(OUT_PATH)/bin
OUT_CAVAN = $(OUT_PATH)/cavan

CC = $(Q)$(CROSS_COMPILE)gcc
LD = $(Q)$(CROSS_COMPILE)ld
AR = $(Q)$(CROSS_COMPILE)ar
MKDIR = $(Q)mkdir -p
RM = $(Q)rm -rf
SED = $(Q)sed
FOR = $(Q)for

CFLAGS +=	-Wall -Wundef -Werror -Wstrict-prototypes -Wno-trigraphs \
			-Werror-implicit-function-declaration -Wno-format-security \
			-fno-strict-aliasing -g -O2 \
			-I$(INCLUDE_PATH) -I. -DCAVAN_ARCH=$(ARCH)
ASFLAGS +=	$(CFLAGS) -D__ASM__
LDFLAGS += -lm -lpthread

ifeq ("$(Q)","@")
  MAKEFLAGS += --no-print-directory
endif

define file_path_convert
$(patsubst %.c,$(2)%$(3),$(notdir $(1)))
endef

define load_cavan_make
$(eval source-app =)
$(eval source-lib =)
$(eval include $(1)/cavan.mk)
$(eval APP_SRC_FILES += $(addprefix $(1)/,$(source-app)))
$(eval LIB_SRC_FILES += $(addprefix $(1)/,$(source-lib)))
endef

define find_source_files
$(if $(wildcard $(1)/cavan.mk),$(call load_cavan_make,$(1)),$(eval $(2) += $(wildcard $(1)/*.c)))
endef

define build_app_action
$(call find_source_files,$(1),APP_SRC_FILES)

$(OUT_APP)/%.o: $(1)/%.c
	@echo "[CC]\t$$@ <= $$^"
	$(CC) $(CFLAGS) -o $$@ -c $$^

$(OUT_CAVAN)/%.c: $(1)/%.c
	@echo "[SED]\t$$^ => $$@"
	$$(eval main-name = $$(patsubst $(1)/%.c,do_cavan_%,$$^))
	$(SED)	-e "s/^\s*int\s\+main\s*\((.*)\)\s*$$$$/int $$(main-name)\1/g" \
			$$^ > $$@
endef

define build_lib_action
$(call find_source_files,$(1),LIB_SRC_FILES)

$(OUT_LIB)/%.o: $(1)/%.c
	@echo "[CC]\t$$@ <= $$^"
	$(CC) $(CFLAGS) -fPIC -o $$@ -c $$^
endef

$(foreach app,$(APP) $(APP_PATH),$(eval $(call build_app_action,$(app))))
$(foreach lib,$(LIB) $(LIB_PATH),$(eval $(call build_lib_action,$(lib))))

LIB_OBJ_FILES = $(call file_path_convert,$(LIB_SRC_FILES),$(OUT_LIB)/,.o)
TARGET_LIBO = $(OUT_LIB)/$(CAVAN_NAME).o
TARGET_LIBSO = $(OUT_LIB)/lib$(CAVAN_NAME).so
TARGET_LIBA = $(OUT_LIB)/lib$(CAVAN_NAME).a

APP_OBJ_FILES = $(call file_path_convert,$(APP_SRC_FILES),$(OUT_APP)/,.o)
TARGET_BINS = $(call file_path_convert,$(APP_SRC_FILES),$(OUT_BIN)/cavan-)
ifeq ($(BUILD_TYPE),debug)
CFLAGS += -DCAVAN_DEBUG
BIN_DEPENDS = $(TARGET_LIBSO)  
APP_DEPENDS =
APP_LDFLAGS += -L$(ROOT_PATH)/$(OUT_LIB) -Wl,-rpath,$(ROOT_PATH)/$(OUT_LIB) -l$(CAVAN_NAME)
else
ifeq ($(BUILD_TYPE),static)
CFLAGS += -DCAVAN_STATIC
APP_DEPENDS = $(TARGET_LIBA)
BIN_DEPENDS =
APP_LDFLAGS += -static -L$(OUT_LIB) -l$(CAVAN_NAME)
else
ifeq ($(BUILD_TYPE),release)
CFLAGS += -DCAVAN_RELEASE
APP_DEPENDS = $(TARGET_LIBO)
BIN_DEPENDS =
APP_LDFLAGS += $(TARGET_LIBO)
else
$(error unknown build type: $(BUILD_TYPE))
endif
endif
endif

CAVAN_SRC_FILES = $(call file_path_convert,$(APP_SRC_FILES),$(OUT_CAVAN)/,.c)
APP_CORE_SRC_FILES = $(wildcard $(APP_CORE_PATH)/*.c)
APP_CORE_OBJ_FILES = $(call file_path_convert,$(APP_CORE_SRC_FILES),$(OUT_CAVAN)/,.o)
CAVAN_OBJ_FILES = $(CAVAN_SRC_FILES:%.c=%.o) $(APP_CORE_OBJ_FILES)
CAVAN_MAP_HEADER = $(OUT_CAVAN)/cavan_map.h
CAVAN_MAP_SOURCE = $(OUT_CAVAN)/cavan_map.c
TARGET_CAVAN = $(OUT_BIN)/$(CAVAN_NAME)-main
CAVAN_CFLAGS = -I$(ROOT_PATH)/$(OUT_CAVAN)

all: $(BUILD_ENTRY)

app: build_env $(BIN_DEPENDS) $(TARGET_BINS)

$(OUT_BIN)/$(CAVAN_NAME)-%: $(OUT_APP)/%.o $(APP_DEPENDS)
	@echo "[LD]\t$@ <= $<"
	$(CC) -o $@ $< $(LDFLAGS) $(APP_LDFLAGS) $(LDFLAGS)

lib: build_env $(TARGET_LIBA) $(TARGET_LIBSO)

liba: build_env $(TARGET_LIBA)

libso: build_env $(TARGET_LIBSO)

$(TARGET_LIBA): $(TARGET_LIBO)
	@echo "[AR]\t$@ <= $^"
	$(AR) cur $@ $^

$(TARGET_LIBSO): $(TARGET_LIBO)
	@echo "[LD]\t$@ <= $^"
	$(CC) -shared -o $@ $^

$(TARGET_LIBO): $(LIB_OBJ_FILES)
	@echo "[LD]\t$@ <= $^"
	$(LD) -o $@ -r $^

cavan: build_env $(BIN_DEPENDS) $(TARGET_CAVAN)

$(TARGET_CAVAN): $(CAVAN_OBJ_FILES) $(APP_DEPENDS)
	@echo "[LD]\t$@ <= $^"
	$(CC) -o $@ $^ $(APP_LDFLAGS) $(LDFLAGS)

$(OUT_CAVAN)/%.o: $(APP_CORE_PATH)/%.c $(CAVAN_MAP_HEADER) $(CAVAN_MAP_SOURCE)
	@echo "[CC]\t$@ <= $<"
	$(CC) $(CFLAGS) $(CAVAN_CFLAGS) -o $@ -c $<

$(CAVAN_MAP_HEADER): $(CAVAN_SRC_FILES)
	$(FOR) app in $(patsubst $(OUT_CAVAN)/%.c,do_cavan_%,$^); \
	do \
		echo "int $${app}(int argc, char *argv[]);"; \
	done > $@

$(CAVAN_MAP_SOURCE): $(CAVAN_SRC_FILES)
	$(FOR) app in $(patsubst $(OUT_CAVAN)/%.c,%,$^); \
	do \
		echo "{\"$${app}\", do_cavan_$${app}},"; \
	done > $@

clean:
	$(RM) $(OUT_PATH) -rfv

build_env:
	$(MKDIR) $(OUT_LIB) $(OUT_APP) $(OUT_BIN) $(OUT_CAVAN)

.PHONY: $(build_env)
