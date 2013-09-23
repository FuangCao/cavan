LOCAL_MODULE_PATH := $(OUT_LIB)/$(LOCAL_MODULE).so

$(eval $(call module_common_action,shared))

$(LOCAL_MODULE_PATH): $(LOCAL_OBJECT)
	$(call link_c_libso)

$(LOCAL_OBJECT): CFLAGS += -fPIC
