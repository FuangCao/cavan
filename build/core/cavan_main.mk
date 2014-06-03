include $(DEFINES_MAKEFILE)

CAVAN_MAIN_FUNC_PREFIX = do_$(CAVAN_NAME)_
CAVAN_MAP_C = $(OUT_CAVAN)/cavan_map.c
CAVAN_MAP_H = $(OUT_CAVAN)/cavan_map.h

CFLAGS += -I$(OUT_CAVAN)

CAVAN_OBJECT = $(call source_to_object,$(OUT_CAVAN),$(CAVAN_SOURCE))
APP_CORE_OBJECT = $(call source_to_object,$(OUT_CAVAN),$(APP_CORE_SOURCE))

TARGET_CAVAN_ELF = $(OUT_BIN)/$(CAVAN_NAME)-main
TARGET_CAVAN_OBJ = $(OUT_CAVAN)/$(CAVAN_NAME).o

all: $(TARGET_CAVAN_ELF)

$(TARGET_CAVAN_ELF): $(TARGET_CAVAN_OBJ)
	$(call build_elf_file)

$(TARGET_CAVAN_OBJ): $(CAVAN_OBJECT)
	$(call build_libo_file)

$(APP_CORE_SOURCE): $(CAVAN_MAP_C) $(CAVAN_MAP_H)

$(CAVAN_MAP_C): $(APP_SOURCE)
	$(call generate_map_source)

$(CAVAN_MAP_H): $(APP_SOURCE)
	$(call generate_map_header)

include $(CAVAN_DEPEND)
include $(CAVAN_SOURCE_DEPEND)

.PHONY: $(CAVAN_MAP_C) $(CAVAN_MAP_H) $(APP_CORE_OBJECT)
