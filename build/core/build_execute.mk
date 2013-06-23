LOCAL_MODULE_PATH := $(OUT_BIN)/cavan-$(LOCAL_MODULE)

ifeq ($(findstring -static,$(LDFLAGS)),)
LOCAL_DEPEND := $(foreach lib,$(LOCAL_LIBRARY),$(OUT_LIB)/$(lib).so)
else
LOCAL_DEPEND := $(foreach lib,$(LOCAL_LIBRARY),$(OUT_LIB)/$(lib).a)
endif

$(LOCAL_MODULE_PATH): APP_LDFLAGS := $(patsubst lib%,-l%,$(LOCAL_LIBRARY))

$(eval $(call module_common_action,execute))

$(LOCAL_MODULE_PATH): $(LOCAL_COBJ) | $(LOCAL_DEPEND)
	@echo "[LD]    $@ <= $^"
	$(Q)$(CC) -o $@ $(LDFLAGS) $^ $(APP_LDFLAGS) -lm -lrt -lpthread
	$(call strip_files,$@)
