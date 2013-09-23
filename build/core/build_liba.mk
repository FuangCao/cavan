LOCAL_MODULE_PATH := $(OUT_LIB)/$(LOCAL_MODULE).a

$(eval $(call module_common_action,static))

$(LOCAL_OUT_PATH)/%.o: CFLAGS += -fPIC

$(LOCAL_MODULE_PATH): $(LOCAL_OBJECT)
	$(call link_c_liba)
