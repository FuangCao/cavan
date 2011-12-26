ROOT_PATH = $(shell pwd)
CAVAN_NAME = cavan
APP_PATH = app
LIB_PATH = lib
INCLUDE_PATH = $(ROOT_PATH)/include
APP_CORE_PATH = $(APP_PATH)/core
MAKEFILE_CAVAN = $(CAVAN_NAME).mk

OUT_PATH = out/$(ARCH)
OUT_LIB = $(OUT_PATH)/lib
OUT_APP = $(OUT_PATH)/app
OUT_CAVAN = $(OUT_PATH)/$(CAVAN_NAME)
OUT_BIN = $(OUT_PATH)/$(BUILD_TYPE)

APP_MARK = $(OUT_APP)/mark

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar
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

define file_path_convert
$(patsubst %.c,$(2)%$(3),$(notdir $(1)))
endef

define load_cavan_make
$(eval source-app =)
$(eval source-lib =)
$(eval include $(1)/$(MAKEFILE_CAVAN))
$(eval APP_SRC_FILES += $(addprefix $(1)/,$(source-app)))
$(eval LIB_SRC_FILES += $(addprefix $(1)/,$(source-lib)))
endef

define find_source_files
$(if $(wildcard $(1)/$(MAKEFILE_CAVAN)),$(call load_cavan_make,$(1)),$(eval $(2) += $(wildcard $(1)/*.c)))
endef

define build_app_action
$(call find_source_files,$(1),APP_SRC_FILES)
$$(OUT_APP)/%.o: $(1)/%.c
	@echo "[CC]\t$$< => $$@"
	$$(Q)$$(CC) $$(CFLAGS) -o $$@ -c $$<
$$(OUT_CAVAN)/%.c: $(1)/%.c
	@echo "[GEN]\t$$< => $$@"
	$$(eval main-name = do_cavan_$$*)
	@sed	-e "s/^\s*int\s\+main\s*\((.*)\)/int $$(main-name)\1/g" \
			-e "s/^\s*void\s\+main\s*\((.*)\)/void $$(main-name)\1/g" \
			$$< > $$@
endef

define build_lib_action
$(call find_source_files,$(1),LIB_SRC_FILES)
$$(OUT_LIB)/%.o: $(1)/%.c
	@echo "[CC]\t$$< => $$@"
	$$(Q)$$(CC) $$(CFLAGS) -fPIC -o $$@ -c $$<
endef

$(foreach app,$(APP) $(APP_PATH),$(eval $(call build_app_action,$(app))))
$(foreach lib,$(LIB) $(LIB_PATH),$(eval $(call build_lib_action,$(lib))))

LIB_OBJ_FILES = $(call file_path_convert,$(LIB_SRC_FILES),$(OUT_LIB)/,.o)
TARGET_LIBO = $(OUT_LIB)/$(CAVAN_NAME).o
TARGET_LIBSO = $(OUT_LIB)/lib$(CAVAN_NAME).so
TARGET_LIBA = $(OUT_LIB)/lib$(CAVAN_NAME).a

APP_OBJ_FILES = $(call file_path_convert,$(APP_SRC_FILES),$(OUT_APP)/,.o)
TARGET_BINS = $(call file_path_convert,$(APP_SRC_FILES),$(OUT_BIN)/$(CAVAN_NAME)-)
ifeq ($(BUILD_TYPE),debug)
CFLAGS += -DCAVAN_DEBUG
APP_DEPENDS = $(TARGET_LIBSO)
APP_LDFLAGS += -L$(ROOT_PATH)/$(OUT_LIB) -Wl,-rpath,$(ROOT_PATH)/$(OUT_LIB) -l$(CAVAN_NAME)
else
ifeq ($(BUILD_TYPE),static)
CFLAGS += -DCAVAN_STATIC
APP_DEPENDS = $(TARGET_LIBA)
APP_LDFLAGS += -static -L$(OUT_LIB) -l$(CAVAN_NAME)
else
ifeq ($(BUILD_TYPE),release)
CFLAGS += -DCAVAN_RELEASE
APP_DEPENDS = $(TARGET_LIBO)
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

$(foreach out-dir,$(OUT_LIB) $(OUT_APP) $(OUT_BIN) $(OUT_CAVAN),$(shell $(MKDIR) $(out-dir)))

all: $(BUILD_ENTRY)

lib: $(TARGET_LIBA) $(TARGET_LIBSO)

liba: $(TARGET_LIBA)

libso: $(TARGET_LIBSO)

cavan: $(TARGET_CAVAN)

ifeq ($(BUILD_TYPE),debug)
app: $(APP_MARK)

$(APP_MARK): $(APP_OBJ_FILES) $(APP_DEPENDS)
	@for app in $(basename $(filter-out $(notdir $(APP_DEPENDS)),$(?F))); \
	do \
		src="$(OUT_APP)/$${app}.o"; \
		dest="$(OUT_BIN)/$(CAVAN_NAME)-$${app}"; \
		echo "[LD]\t$${src} => $${dest}"; \
		$(CC) -o $${dest} $${src} $(APP_LDFLAGS) $(LDFLAGS); \
	done
	@echo "$?" > $@

$(TARGET_CAVAN): $(CAVAN_OBJ_FILES) $(APP_DEPENDS)
	$(eval modify-files = $(filter-out $(APP_DEPENDS),$?))
	@test -z "$(modify-files)" || \
	{ \
		obj_files="$(filter-out $(APP_DEPENDS),$^)"; \
		echo "[LD]\t$@ <= $${obj_files}"; \
		$(CC) -o $@ $${obj_files} $(APP_LDFLAGS) $(LDFLAGS); \
	}
else
app: $(TARGET_BINS)

$(OUT_BIN)/$(CAVAN_NAME)-%: $(OUT_APP)/%.o $(APP_DEPENDS)
	@echo "[LD]\t$< => $@"
	$(Q)$(CC) -o $@ $< $(APP_LDFLAGS) $(LDFLAGS)

$(TARGET_CAVAN): $(CAVAN_OBJ_FILES) $(APP_DEPENDS)
	$(eval obj-files = $(filter-out $(APP_DEPENDS),$^))
	@echo "[LD]\t$@ <= $(obj-files)"
	$(Q)$(CC) -o $@ $(obj-files) $(APP_LDFLAGS) $(LDFLAGS)
endif

$(TARGET_LIBA): $(TARGET_LIBO)
	@echo "[AR]\t$@ <= $^"
	$(Q)$(AR) cur $@ $^

$(TARGET_LIBSO): $(TARGET_LIBO)
	@echo "[LD]\t$@ <= $^"
	$(Q)$(CC) -shared -o $@ $^

$(TARGET_LIBO): $(LIB_OBJ_FILES)
	@echo "[LD]\t$@ <= $^"
	$(Q)$(LD) -o $@ -r $^

$(OUT_CAVAN)/%.o: $(APP_CORE_PATH)/%.c $(CAVAN_MAP_HEADER) $(CAVAN_MAP_SOURCE)
	@echo "[CC]\t$< => $@"
	$(Q)$(CC) $(CFLAGS) $(CAVAN_CFLAGS) -o $@ -c $<

$(OUT_CAVAN)/%.o: $(OUT_CAVAN)/%.c
	@echo "[CC]\t$^ => $@"
	$(Q)$(CC) $(CFLAGS) $(CAVAN_CFLAGS) -o $@ -c $^

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
