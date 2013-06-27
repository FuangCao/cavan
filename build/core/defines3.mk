define clear_vars
$(eval LOCAL_C_SOURCE :=)
$(eval LOCAL_C_INCLUDE :=)
$(eval LOCAL_CFLAGS :=)
$(eval LOCAL_LDFLAGS :=)
$(eval LOCAL_LIBRARY :=)
$(eval LOCAL_MODULE :=)
endef

define build_sub_module
$(call clear_vars)
$(eval LOCAL_PATH := $(ROOT_PATH)/$(1))
$(eval include $(LOCAL_PATH)/cavan.mk)
endef

define module_common_action
ifeq ($$(LOCAL_MODULE),)
$$(error Please give module name for $$(LOCAL_PATH))
endif

LOCAL_OUT_PATH := $$(OUT_OBJ)/$$(LOCAL_MODULE)_$(1)
LOCAL_COBJ := $$(patsubst %.c,$$(LOCAL_OUT_PATH)/%.o,$$(LOCAL_C_SOURCE))
$$(LOCAL_MODULE_PATH): CFLAGS := $$(CFLAGS) $$(LOCAL_CFLAGS) $$(addprefix -I,$$(LOCAL_C_INCLUDE))

$$(LOCAL_OUT_PATH):
	$$(Q)$$(MKDIR) $$@

MODULES := $$(MODULES) $$(LOCAL_MODULE_PATH)
endef

define build_c_object
@echo "[CC]    $< => $@"
$(Q)$(CC) -o $@ $(CFLAGS) $(1) -c $<
endef

define search_all_files
$(patsubst $(LOCAL_PATH)/%,%,$(wildcard $(LOCAL_PATH)/$(1)))
endef

define strip_files
$(Q)$(STRIP) -s $(1)
endef

define build_as_execute
LOCAL_C_SOURCE := $(1)
LOCAL_MODULE := $(basename $(notdir $(1)))
include $(BUILD_EXECUTE)
endef
