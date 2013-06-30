LOCAL_MODULE_PATH := $(OUT_LIB)/$(LOCAL_MODULE).so

$(eval $(call module_common_action,shared))

$(LOCAL_OUT_PATH)/%.o: CFLAGS += -fPIC

$(LOCAL_MODULE_PATH): $(LOCAL_COBJ)
	$(call link_c_libso)
