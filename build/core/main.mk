ifeq ("$(ARCH)","")
ARCH = x86
CROSS_COMPILE =
else
ifeq ("$(ARCH)","arm")
ifeq ("$(CROSS_COMPILE)","")
CROSS_COMPILE = arm-linux-
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

CAVAN_ROOT = $(shell pwd)
OUT_DIR = out
LIB_DIR = lib
APP_DIR = app
INCLUDE_DIR = include
BUILD_DIR = build
BUILD_CORE = $(BUILD_DIR)/core
APP_CORE = $(APP_DIR)/core

TARGET_OUT = $(OUT_DIR)/$(ARCH)
OUT_LIB = $(TARGET_OUT)/lib
OUT_ELF = $(TARGET_OUT)/bin
OUT_APP = $(TARGET_OUT)/app
OUT_CAVAN = $(TARGET_OUT)/cavan

DEPEND_NAME = depend.mk
LIB_DEPEND = $(OUT_LIB)/$(DEPEND_NAME)
APP_DEPEND = $(OUT_APP)/$(DEPEND_NAME)
ELF_DEPEND = $(OUT_ELF)/$(DEPEND_NAME)
CAVAN_DEPEND = $(OUT_CAVAN)/$(DEPEND_NAME)
CAVAN_SOURCE_DEPEND = $(OUT_CAVAN)/source_$(DEPEND_NAME)
APP_CORE_DEPEND = $(OUT_CAVAN)/app_core_$(DEPEND_NAME)

CAVAN_NAME = cavan
TARGET_OBJ = $(OUT_LIB)/lib$(CAVAN_NAME).o
TARGET_LIBA = $(OUT_LIB)/lib$(CAVAN_NAME).a
TARGET_LIBSO = $(OUT_LIB)/lib$(CAVAN_NAME).so
TARGET_CAVAN_ELF = $(OUT_CAVAN)/$(CAVAN_NAME)
TARGET_CAVAN_OBJ = $(TARGET_CAVAN_ELF).o

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
			-I$(INCLUDE_DIR) -I$(OUT_CAVAN) -I. -DARCH=$(ARCH)
ASFLAGS +=	$(CFLAGS) -D__ASM__

ifeq ("$(findstring release,$(BUILD_TYPE))","")
ifeq ("$(findstring static,$(BUILD_TYPE))","")
LOCAL_LDFLAGS += -Wl,-rpath,$(CAVAN_ROOT)/$(OUT_LIB)
else
ELF_DEPEND_LIB = $(TARGET_LIBA)
LOCAL_LDFLAGS += -static
endif
LOCAL_LDFLAGS += -L$(OUT_LIB) -l$(CAVAN_NAME)
else
LDFLAGS += $(TARGET_OBJ)
ELF_DEPEND_LIB = $(TARGET_OBJ)
endif
LDFLAGS := -s $(LOCAL_LDFLAGS) $(LDFLAGS)

ifeq ("$(origin APP)","command line")
ifeq ("$(wildcard $(APP)/cavan.mk)","$(APP)/cavan.mk")
include $(APP)/cavan.mk
APP_SOURCE += $(addprefix $(APP)/,$(source-app))
LIB_SOURCE += $(addprefix $(APP)/,$(source-lib))
else
APP_SOURCE += $(wildcard $(APP)/*.c)
endif
endif

ifeq ("$(origin LIB)","command line")
ifeq ("$(wildcard $(LIB)/cavan.mk)","$(LIB)/cavan.mk")
include $(LIB)/cavan.mk
APP_SOURCE += $(addprefix $(LIB)/,$(source-app))
LIB_SOURCE += $(addprefix $(LIB)/,$(source-lib))
else
LIB_SOURCE += $(wildcard $(LIB)/*.c)
endif
endif

LIB_SOURCE += $(wildcard $(LIB_DIR)/*.c)
LIB_OBJECT = $(patsubst %.c,$(OUT_LIB)/%.o,$(notdir $(LIB_SOURCE)))
APP_SOURCE  += $(wildcard $(APP_DIR)/*.c)
APP_OBJECT = $(patsubst %.c,$(OUT_APP)/%.o,$(notdir $(APP_SOURCE)))
ifeq ("$(strip $(ELF_PREFIX))","")
ELF_OBJECT = $(patsubst %.c,$(OUT_ELF)/%,$(notdir $(APP_SOURCE)))
else
ELF_OBJECT = $(patsubst %.c,$(OUT_ELF)/$(ELF_PREFIX)-%,$(notdir $(APP_SOURCE)))
endif
HEADER_FILES = $(wildcard $(INCLUDE_DIR)/cavan/*.h) $(INCLUDE_DIR)/cavan.h

CAVAN_SOURCE = $(foreach fn,$(APP_SOURCE),$(OUT_CAVAN)/$(notdir $(fn)))
CAVAN_OBJECT = $(CAVAN_SOURCE:%.c=%.o)
APP_CORE_SOURCE = $(wildcard $(APP_CORE)/*.c)
APP_CORE_OBJECT = $(patsubst %.c,$(OUT_CAVAN)/%.o,$(notdir $(APP_CORE_SOURCE)))
CAVAN_MAP = $(OUT_CAVAN)/cavan_map.h
CAVAN_CMD = $(OUT_CAVAN)/cavan_cmd.h

ifeq ("$(Q)","@")
MAKEFLAGS += --no-print-directory
endif

$(info ============================================================)
$(info CAVAN_ROOT = $(CAVAN_ROOT))
$(info ARCH = $(ARCH))
$(info CROSS_COMPILE = $(CROSS_COMPILE))
$(info BUILD_TYPE = $(BUILD_TYPE))
$(info ============================================================)

export CC LD AR CFLAGS LDFLAGS
export CAVAN_ROOT OUT_DIR LIB_DIR APP_DIR INCLUDE_DIR BUILD_DIR BUILD_CORE
export TARGET_OUT OUT_LIB OUT_ELF OUT_APP OUT_CAVAN
export LIB_DEPEND APP_DEPEND ELF_DEPEND CAVAN_DEPEND CAVAN_SOURCE_DEPEND APP_CORE_DEPEND
export TARGET_OBJ TARGET_LIBA TARGET_LIBSO TARGET_CAVAN_ELF TARGET_CAVAN_OBJ
export LIB_SOURCE APP_SOURCE HEADER_FILES CAVAN_SOURCE CAVAN_MAP CAVAN_CMD APP_CORE_SOURCE
export LIB_OBJECT APP_OBJECT ELF_OBJECT CAVAN_OBJECT APP_CORE_OBJECT
export APPS_MAKEFILE LIBS_MAKEFILE DEFINES_MAKEFILE TOGETHER_MAKEFILE

all: app

app: lib $(OUT_APP) $(OUT_ELF) $(APP_DEPEND) $(ELF_DEPEND)
	$(Q)make -f $(APPS_MAKEFILE)

lib: $(OUT_LIB) $(LIB_DEPEND)
	$(Q)make -f $(LIBS_MAKEFILE)

one join together cavan: lib $(OUT_CAVAN) $(CAVAN_SOURCE_DEPEND) $(CAVAN_DEPEND) $(APP_CORE_DEPEND)
	$(Q)make -f $(TOGETHER_MAKEFILE)

$(CAVAN_DEPEND): $(APP_SOURCE)
	$(call build_cavan_depend,$(CAVAN_MAP),$(CAVAN_CMD))

$(CAVAN_SOURCE_DEPEND): $(APP_SOURCE)
	$(call build_cavan_source_depend)

$(APP_CORE_DEPEND): $(APP_CORE_SOURCE)
	$(call build_obj_depend)

$(LIB_DEPEND): $(LIB_SOURCE)
	$(call build_obj_depend)

$(APP_DEPEND): $(APP_SOURCE)
	$(call build_obj_depend)

$(ELF_DEPEND): $(APP_SOURCE)
	$(call build_elf_depend,$(OUT_APP),$(ELF_PREFIX))

$(APP_SOURCE): $(HEADER_FILES) $(ELF_DEPEND_LIB)
	$(call touch_file)

$(LIB_SOURCE): $(HEADER_FILES)
	$(call touch_file)

$(OUT_LIB) $(OUT_ELF) $(OUT_APP) $(OUT_CAVAN): $(TARGET_OUT)
	$(call make_directory)

$(TARGET_OUT): $(OUT_DIR)
	$(call make_directory)

$(OUT_DIR):
	$(call make_directory)

clean clean-bin:
	$(call remove_file,$(OUT_ELF))
	$(call remove_file,$(TARGET_CAVAN_ELF))

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

.PHONY: lib app $(APP_DEPEND) $(LIB_DEPEND) $(ELF_DEPEND) $(CAVAN_DEPEND) $(CAVAN_SOURCE_DEPEND) $(APP_CORE_DEPEND)

include $(DEFINES_MAKEFILE)
