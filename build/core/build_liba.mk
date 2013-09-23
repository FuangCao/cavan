LOCAL_MODULE_PATH := $(OUT_LIB)/$(LOCAL_MODULE).a

$(eval $(call module_common_action,static))

$(LOCAL_MODULE_PATH): $(LOCAL_OBJECT)
	$(call link_c_liba)

$(LOCAL_OBJECT): CFLAGS += -fPIC
