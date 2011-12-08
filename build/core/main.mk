CAVAN_NAME = cavan
DEPEND_NAME = depend.mk
CAVAN_ROOT = $(shell pwd)
CAVAN_BUILD_ARCH = $(shell uname -m)

OUT_DIR = out
LIB_DIR = lib
APP_DIR = app
INCLUDE_DIR = include
BUILD_DIR = build
BUILD_CORE = $(BUILD_DIR)/core
APP_CORE = $(APP_DIR)/core

CONFIG_FILE_PATH = .config

ifneq ($(wildcard $(CONFIG_FILE_PATH)),)
include $(CONFIG_FILE_PATH)
endif

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
ifeq ("$(ARCH)","x86")
  CROSS_COMPILE =
endif
endif
endif

ifeq ("$(origin BT)","command line")
  BUILD_TYPE = $(BT)
else
  ifeq ("$(BUILD_TYPE)","")
    BUILD_TYPE = release
  endif
endif

TARGET_OUT = $(OUT_DIR)/$(ARCH)
OUT_LIB = $(TARGET_OUT)/lib
TARGET_LIB_OBJ = $(OUT_LIB)/lib$(CAVAN_NAME).o
TARGET_LIBA = $(OUT_LIB)/lib$(CAVAN_NAME).a
TARGET_LIBSO = $(OUT_LIB)/lib$(CAVAN_NAME).so
TARGET_LIB_ALL = $(TARGET_LIB_OBJ) $(TARGET_LIBA) $(TARGET_LIBSO)

ifeq ("$(findstring release,$(BUILD_TYPE))","")
  ifeq ("$(findstring static,$(BUILD_TYPE))","")
    BUILD_TYPE = dynamically
    LOCAL_LDFLAGS += -Wl,-rpath,$(CAVAN_ROOT)/$(OUT_LIB)
    APP_DEPEND_LIB = $(TARGET_LIBSO)
  else
    BUILD_TYPE = static
    LOCAL_LDFLAGS += -static
    APP_DEPEND_LIB = $(TARGET_LIBA)
  endif
   LOCAL_LDFLAGS += -L$(OUT_LIB) -l$(CAVAN_NAME)
else
  BUILD_TYPE = release
  LOCAL_LDFLAGS += $(TARGET_LIB_OBJ)
  APP_DEPEND_LIB = $(TARGET_LIB_OBJ)
endif

OUT_ELF = $(TARGET_OUT)/$(BUILD_TYPE)
OUT_APP = $(TARGET_OUT)/app
OUT_CAVAN = $(TARGET_OUT)/cavan

LIB_DEPEND = $(OUT_LIB)/$(DEPEND_NAME)
APP_DEPEND = $(OUT_APP)/$(DEPEND_NAME)
ELF_DEPEND = $(OUT_ELF)/$(DEPEND_NAME)
CAVAN_DEPEND = $(OUT_CAVAN)/$(DEPEND_NAME)
CAVAN_SOURCE_DEPEND = $(OUT_CAVAN)/source_$(DEPEND_NAME)

APPS_MAKEFILE = $(BUILD_CORE)/application.mk
LIBS_MAKEFILE = $(BUILD_CORE)/library.mk
DEFINES_MAKEFILE = $(BUILD_CORE)/defines.mk
TOGETHER_MAKEFILE = $(BUILD_CORE)/together.mk

CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar

CFLAGS +=	-Wall -Wundef -Werror -Wstrict-prototypes -Wno-trigraphs \
			-Werror-implicit-function-declaration -Wno-format-security \
			-fno-strict-aliasing -g -O2 \
			-I$(INCLUDE_DIR) -I. -DARCH=$(ARCH)
ASFLAGS +=	$(CFLAGS) -D__ASM__
LDFLAGS := -s $(LOCAL_LDFLAGS) $(LDFLAGS)

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
export TARGET_OUT OUT_LIB OUT_ELF OUT_APP OUT_CAVAN
export LIB_DEPEND APP_DEPEND ELF_DEPEND CAVAN_DEPEND CAVAN_SOURCE_DEPEND
export CAVAN_NAME TARGET_LIB_OBJ TARGET_LIBA TARGET_LIBSO
export LIB_SOURCE APP_SOURCE HEADER_FILES CAVAN_SOURCE APP_CORE_SOURCE
export APPS_MAKEFILE LIBS_MAKEFILE DEFINES_MAKEFILE TOGETHER_MAKEFILE

include $(DEFINES_MAKEFILE)

all: $(BUILD_ENTRY)
	$(Q)echo "Compile is OK"

app: $(OUT_APP) $(OUT_ELF) $(APP_DEPEND_LIB) $(APP_SOURCE)
	$(call generate_obj_depend,$(APP_DEPEND),$(APP_SOURCE))
	$(call generate_elf_depend,$(ELF_DEPEND),$(OUT_APP),$(APP_SOURCE))
	$(Q)+make -f $(APPS_MAKEFILE)

lib: $(TARGET_LIB_ALL)

$(TARGET_LIB_ALL): $(OUT_LIB) $(LIB_SOURCE)
	$(call generate_obj_depend,$(LIB_DEPEND),$(LIB_SOURCE))
	$(Q)+make -f $(LIBS_MAKEFILE) $@

one join together cavan: $(OUT_CAVAN) $(APP_DEPEND_LIB)
	$(call generate_src_depend,$(CAVAN_SOURCE_DEPEND),$(APP_SOURCE))
	$(call generate_cavan_obj_depend,$(CAVAN_DEPEND),$(CAVAN_SOURCE))
	$(Q)+make -f $(TOGETHER_MAKEFILE)

config:
	$(Q){ \
		echo "ARCH = $(ARCH)"; \
		echo "CROSS_COMPILE = $(CROSS_COMPILE)"; \
		echo "BUILD_ENTRY = $(BUILD_ENTRY)"; \
		echo BUILD_TYPE = $(BUILD_TYPE); \
	} > $(CONFIG_FILE_PATH)

$(OUT_LIB) $(OUT_ELF) $(OUT_APP) $(OUT_CAVAN): $(TARGET_OUT)
	$(call make_directory)

$(TARGET_OUT): $(OUT_DIR)
	$(call make_directory)

$(OUT_DIR):
	$(call make_directory)

clean clean-bin:
	$(call remove_file,$(OUT_ELF))

clean-app:
	$(call remove_file,$(OUT_APP))

clean-lib:
	$(call remove_file,$(OUT_LIB))

clean-cavan:
	$(call remove_file,$(OUT_CAVAN))

clean-all:
	$(call remove_file,$(TARGET_OUT))

distclean:
	$(call remove_file,$(OUT_DIR))
