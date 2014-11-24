LOCAL_MODULE_PATH := $(OUT_BIN)/cavan-$(LOCAL_MODULE)

$(eval $(call module_common_action,execute))

$(eval $(call link_c_execute_action))
