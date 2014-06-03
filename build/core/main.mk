CAVAN_NAME = cavan
DEPEND_NAME = depend.mk
CAVAN_ROOT = $(shell pwd)
CAVAN_BUILD_ARCH = $(shell uname -m)
ELF_PREFIX = $(CAVAN_NAME)

CONFIG_FILE_PATH = .config

ifneq ($(wildcard $(CONFIG_FILE_PATH)),)
include $(CONFIG_FILE_PATH)
endif

OUT_DIR = out/$(ARCH)/$(BUILD_TYPE)
LIB_DIR = lib
APP_DIR = app
CONFIG_DIR = config
INCLUDE_DIR = include
BUILD_DIR = build
BUILD_CORE = $(BUILD_DIR)/core
APP_CORE = $(APP_DIR)/core

ifeq ("$(ARCH)","")
  ARCH = $(CAVAN_BUILD_ARCH)
endif

ifeq ("$(ARCH)","$(CAVAN_BUILD_ARCH)")
  CROSS_COMPILE =
else
ifeq ("$(ARCH)","arm")
    ifeq ("$(CROSS_COMPILE)","")
      CROSS_COMPILE = arm-linux-
    endif
else
ifeq ("$(ARCH)","host")
  CROSS_COMPILE =
endif
endif
endif

OUT_LIB = $(OUT_DIR)/lib
TARGET_LIB_OBJ = $(OUT_LIB)/lib$(CAVAN_NAME).o
TARGET_LIBA = $(OUT_LIB)/lib$(CAVAN_NAME).a
TARGET_LIBSO = $(OUT_LIB)/lib$(CAVAN_NAME).so
TARGET_LIB_ALL = $(TARGET_LIB_OBJ) $(TARGET_LIBA) $(TARGET_LIBSO)

ifeq ("$(findstring release,$(BUILD_TYPE))","")
  ifeq ("$(findstring static,$(BUILD_TYPE))","")
    BUILD_TYPE = debug
    CFLAGS += -DCAVAN_DEBUG
    LOCAL_LDFLAGS += -Wl,-rpath,$(CAVAN_ROOT)/$(OUT_LIB)
    APP_DEPEND_LIB = $(TARGET_LIBSO)
  else
    BUILD_TYPE = static
    CFLAGS += -DCAVAN_STATIC
    LOCAL_LDFLAGS += -static
    APP_DEPEND_LIB = $(TARGET_LIBA)
  endif
   LOCAL_LDFLAGS += -L$(OUT_LIB) -l$(CAVAN_NAME)
else
  BUILD_TYPE = release
  CFLAGS += -DCAVAN_RELEASE
  LOCAL_LDFLAGS += $(TARGET_LIB_OBJ)
  APP_DEPEND_LIB = $(TARGET_LIB_OBJ)
endif

OUT_BIN = $(OUT_DIR)/bin
OUT_APP = $(OUT_DIR)/app
OUT_CAVAN = $(OUT_DIR)/cavan

LIB_DEPEND = $(OUT_LIB)/$(DEPEND_NAME)
APP_DEPEND = $(OUT_APP)/$(DEPEND_NAME)
ELF_DEPEND = $(OUT_BIN)/$(DEPEND_NAME)
CAVAN_DEPEND = $(OUT_CAVAN)/$(DEPEND_NAME)
CAVAN_SOURCE_DEPEND = $(OUT_CAVAN)/source_$(DEPEND_NAME)

APPS_MAKEFILE = $(BUILD_CORE)/application.mk
LIBS_MAKEFILE = $(BUILD_CORE)/library.mk
DEFINES_MAKEFILE = $(BUILD_CORE)/defines.mk
CAVAN_MAIN_MAKEFILE = $(BUILD_CORE)/cavan_main.mk

CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar

CFLAGS +=	-Wall -Wundef -Werror -Wstrict-prototypes -Wno-trigraphs \
			-Werror-implicit-function-declaration -Wno-format-security \
			-fno-strict-aliasing -g -O2 \
			-I$(INCLUDE_DIR) -I. -DCAVAN_ARCH=$(ARCH)
ASFLAGS +=	$(CFLAGS) -D__ASM__
LDFLAGS := -s $(LOCAL_LDFLAGS) $(LDFLAGS) -lm -lpthread

define import_extra_code
ifeq ($(wildcard $1/cavan.mk),)
  $(eval $2 += $(wildcard $1/*.c))
else
  source-app =
  source-lib =
  include $1/cavan.mk
  APP_SOURCE += $(addprefix $1/,$(source-app))
  LIB_SOURCE += $(addprefix $1/,$(source-lib))
endif
endef

$(foreach path,$(APP),$(eval $(call import_extra_code,$(path),APP_SOURCE)))
$(foreach path,$(LIB),$(eval $(call import_extra_code,$(path),LIB_SOURCE)))

LIB_SOURCE += $(wildcard $(LIB_DIR)/*.c)
APP_SOURCE  += $(wildcard $(APP_DIR)/*.c)
APP_CORE_SOURCE = $(wildcard $(APP_CORE)/*.c)
HEADER_FILES = $(wildcard $(INCLUDE_DIR)/cavan/*.h) $(INCLUDE_DIR)/cavan.h

CAVAN_SOURCE = $(addprefix $(OUT_CAVAN)/,$(notdir $(APP_SOURCE)))
CAVAN_SOURCE += $(APP_CORE_SOURCE)

CAVAN_CONFIGS = $(notdir $(wildcard config/*))

ifeq ("$(Q)","@")
  MAKEFLAGS += --no-print-directory
endif

$(info ============================================================)
$(info CAVAN_ROOT = $(CAVAN_ROOT))
$(info ARCH = $(ARCH))
$(info CROSS_COMPILE = $(CROSS_COMPILE))
$(info BUILD_TYPE = $(BUILD_TYPE))
$(info CFLAGS = $(CFLAGS))
$(info LDFLAGS = $(LDFLAGS))
$(info ============================================================)

export BUILD_TYPE CROSS_COMPILE ARCH ELF_PREFIX Q
export CC LD AR CFLAGS LDFLAGS
export CAVAN_ROOT OUT_DIR LIB_DIR APP_DIR INCLUDE_DIR BUILD_DIR BUILD_CORE
export OUT_DIR OUT_LIB OUT_BIN OUT_APP OUT_CAVAN
export LIB_DEPEND APP_DEPEND ELF_DEPEND CAVAN_DEPEND CAVAN_SOURCE_DEPEND
export CAVAN_NAME TARGET_LIB_OBJ TARGET_LIBA TARGET_LIBSO
export LIB_SOURCE APP_SOURCE HEADER_FILES CAVAN_SOURCE APP_CORE_SOURCE
export APPS_MAKEFILE LIBS_MAKEFILE DEFINES_MAKEFILE CAVAN_MAIN_MAKEFILE

include $(DEFINES_MAKEFILE)

all: $(BUILD_ENTRY)
	$(Q)echo "Compile is OK"

app: build_env $(APP_DEPEND_LIB) $(APP_SOURCE)
	$(call generate_obj_depend,$(APP_DEPEND),$(APP_SOURCE))
	$(call generate_elf_depend,$(ELF_DEPEND),$(OUT_APP),$(APP_SOURCE))
	$(Q)+make -f $(APPS_MAKEFILE)

lib: build_env $(TARGET_LIB_ALL)

$(TARGET_LIB_ALL): $(LIB_SOURCE)
	$(call generate_obj_depend,$(LIB_DEPEND),$(LIB_SOURCE))
	$(Q)+make -f $(LIBS_MAKEFILE) $@

one join cavan_main cavan: build_env $(APP_DEPEND_LIB)
	$(call generate_src_depend,$(CAVAN_SOURCE_DEPEND),$(APP_SOURCE))
	$(call generate_cavan_obj_depend,$(CAVAN_DEPEND),$(CAVAN_SOURCE))
	$(Q)+make -f $(CAVAN_MAIN_MAKEFILE)

$(CAVAN_CONFIGS):
	$(Q)cp $(CONFIG_DIR)/$@ $(CONFIG_FILE_PATH)

config:
	$(Q){ \
		echo "ARCH = $(ARCH)"; \
		echo "CROSS_COMPILE = $(CROSS_COMPILE)"; \
		echo "BUILD_ENTRY = $(BUILD_ENTRY)"; \
		echo BUILD_TYPE = $(BUILD_TYPE); \
	} > $(CONFIG_FILE_PATH)

$(OUT_LIB) $(OUT_BIN) $(OUT_APP) $(OUT_CAVAN): $(OUT_DIR)
	$(call make_directory)

$(OUT_DIR):
	$(call make_directory)

build_env: $(OUT_LIB) $(OUT_BIN) $(OUT_APP) $(OUT_CAVAN)

clean clean-bin:
	$(call remove_file,$(OUT_BIN))

clean-app:
	$(call remove_file,$(OUT_APP))

clean-lib:
	$(call remove_file,$(OUT_LIB))

clean-cavan:
	$(call remove_file,$(OUT_CAVAN))

clean-all:
	$(call remove_file,$(OUT_DIR))

distclean:
	$(call remove_file,$(OUT_DIR))

.PHONE: build_env
