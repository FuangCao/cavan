LOCAL_MODULE_PATH := $(OUT_LIB)/$(LOCAL_MODULE).a

$(eval $(call module_common_action,static))

$(LOCAL_MODULE_PATH): $(LOCAL_COBJ)
	@echo "[AR]    $@ <= $^"
	$(Q)$(AR) cur $@ $^
