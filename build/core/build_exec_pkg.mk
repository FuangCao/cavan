LOCAL_MODULE_PATH := $(OUT_BIN)/cavan-$(LOCAL_MODULE)

$(eval $(call module_common_action,package))
$(eval $(call link_c_execute))
