LOCAL_MODULE_PATH := $(OUT_BIN)/cavan-$(LOCAL_MODULE)

$(eval $(call module_common_action,package))

ifeq ($(findstring -static,$(LDFLAGS)),)
LOCAL_DEPEND := $(foreach lib,$(LOCAL_LIBRARY),$(OUT_LIB)/$(lib).so)
else
LOCAL_DEPEND := $(foreach lib,$(LOCAL_LIBRARY),$(OUT_LIB)/$(lib).a)
endif

APP_CORE_OUT_PATH := $(LOCAL_OUT_PATH)/core
APP_CORE_SRC := $(wildcard $(APP_CORE_PATH)/*.c)
APP_CORE_OBJ := $(patsubst %.c,$(APP_CORE_OUT_PATH)/%.o,$(notdir $(APP_CORE_SRC)))
APP_CORE_MAP_C := $(APP_CORE_OUT_PATH)/cavan_map.c
APP_CORE_MAP_H := $(APP_CORE_OUT_PATH)/cavan_map.h

$(LOCAL_MODULE_PATH): APP_LDFLAGS := $(patsubst lib%,-l%,$(LOCAL_LIBRARY))

$(APP_CORE_OUT_PATH)/%.o: $(APP_CORE_PATH)/*.c | $(APP_CORE_MAP_C) $(APP_CORE_MAP_H)
	$(call build_c_object,-I$(APP_CORE_OUT_PATH))

$(LOCAL_OUT_PATH)/%.o: $(LOCAL_PATH)/%.c | $(LOCAL_OUT_PATH)
	$(call build_c_object,-Dmain=$(patsubst %.c,do_cavan_%,$(notdir $<)))

$(APP_CORE_MAP_C): $(LOCAL_COBJ) | $(APP_CORE_OUT_PATH)
	@echo "[GEN]   $@ <= $^"
	@for obj in $(basename $(notdir $^)); \
	do \
		echo "{\"$${obj}\", do_cavan_$${obj}},"; \
	done > $@

$(APP_CORE_MAP_H): $(LOCAL_COBJ) | $(APP_CORE_OUT_PATH)
	@echo "[GEN]   $@ <= $^"
	@for obj in $(basename $(notdir $^)); \
	do \
		echo "int do_cavan_$${obj}(int argc, char *argv[]);"; \
	done > $@

$(APP_CORE_OUT_PATH):
	@mkdir -p $@

$(LOCAL_MODULE_PATH): $(LOCAL_COBJ) $(APP_CORE_OBJ) | $(LOCAL_DEPEND)
	@echo "[LD]    $@ <= $^"
	$(Q)$(CC) -o $@ $(LDFLAGS) $^ $(APP_LDFLAGS) -lm -lrt -lpthread
	$(call strip_files,$@)
