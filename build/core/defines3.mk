define clear_vars
$(eval LOCAL_SOURCE :=)
$(eval LOCAL_INCLUDE :=)
$(eval LOCAL_CFLAGS :=)
$(eval LOCAL_LDFLAGS :=)
$(eval LOCAL_LIBRARY :=)
$(eval LOCAL_MODULE :=)
endef

define build_sub_module
$(call clear_vars)
$(eval LOCAL_PATH := $(1))
$(eval LOCAL_MAKEFILE := $(wildcard $(ROOT_PATH)/$(LOCAL_PATH)/cavan.mk))
$(if $(LOCAL_MAKEFILE),$(eval include $(LOCAL_MAKEFILE)),$(error no cavan.mk found in $(LOCAL_PATH)))
endef

define module_package_action
APP_CORE_OUT_PATH := $$(LOCAL_OUT_PATH)_core
APP_CORE_SRC := $$(wildcard $$(APP_CORE_PATH)/*.c)
APP_CORE_MAP_C := $$(APP_CORE_OUT_PATH)/cavan_map.c
APP_CORE_MAP_H := $$(APP_CORE_OUT_PATH)/cavan_map.h

$$(APP_CORE_OUT_PATH)/%.o: $$(APP_CORE_PATH)/%.c | $$(APP_CORE_MAP_C) $$(APP_CORE_MAP_H)
	$$(call build_c_object,-I$$(APP_CORE_OUT_PATH))

$$(APP_CORE_MAP_C): $$(LOCAL_OBJECT) | $$(APP_CORE_OUT_PATH)
	@echo "[GEN]   $$@ <= $$(notdir $$^)"
	@for obj in $$(basename $$(notdir $$^)); \
	do \
		echo "{ \"$$$${obj}\", do_cavan_$$$${obj} },"; \
	done > $$@

$$(APP_CORE_MAP_H): $$(LOCAL_OBJECT) | $$(APP_CORE_OUT_PATH)
	@echo "[GEN]   $$@ <= $$(notdir $$^)"
	@for obj in $$(basename $$(notdir $$^)); \
	do \
		echo "int do_cavan_$$$${obj}(int argc, char *argv[]);"; \
	done > $$@

$$(APP_CORE_OUT_PATH):
	$$(Q)$$(MKDIR) $$@

LOCAL_OBJECT += $$(patsubst %.c,$$(APP_CORE_OUT_PATH)/%.o,$$(notdir $$(APP_CORE_SRC)))
$$(LOCAL_OUT_PATH)/%.o: CFLAGS += -Dmain=$$(patsubst %.o,do_cavan_%,$$(notdir $$@))
$$(LOCAL_OUT_PATH)/%.o: CPPFLAGS += -Dmain=$$(patsubst %.o,do_cavan_%,$$(notdir $$@))
endef

define module_common_action
ifeq ($$(LOCAL_MODULE),)
$$(error Please give module name for $$(LOCAL_PATH))
endif

LOCAL_OUT_PATH := $$(OUT_OBJ)/$$(LOCAL_MODULE)_$(1)
LOCAL_OBJECT := $$(patsubst %,$$(LOCAL_OUT_PATH)/%.o,$$(basename $$(LOCAL_SOURCE)))
PRIVATE_CFLAGS := $$(LOCAL_CFLAGS) $$(addprefix -I,$$(LOCAL_INCLUDE) $$(LOCAL_PATH))

$$(LOCAL_MODULE_PATH): CFLAGS += $$(PRIVATE_CFLAGS)

ifneq ($$(filter %.cpp %.cxx %.cc,$$(LOCAL_SOURCE)),)
$$(LOCAL_MODULE_PATH): CPPFLAGS += $$(PRIVATE_CFLAGS)
$$(LOCAL_MODULE_PATH): LD := $(CPP)
else
$$(LOCAL_MODULE_PATH): LD := $(CC)
endif

$$(if $$(filter package,$(1)),$$(eval $$(call module_package_action)))

$$(LOCAL_OUT_PATH)/%.o: $$(LOCAL_PATH)/%.c | $$(LOCAL_OUT_PATH)
	$$(Q)$$(MKDIR) $$(@D)
	$$(call build_c_object)

$$(LOCAL_OUT_PATH)/%.o: $$(LOCAL_PATH)/%.cc | $$(LOCAL_OUT_PATH)
	$$(Q)$$(MKDIR) $$(@D)
	$$(call build_cpp_object)

$$(LOCAL_OUT_PATH)/%.o: $$(LOCAL_PATH)/%.cpp | $$(LOCAL_OUT_PATH)
	$$(Q)$$(MKDIR) $$(@D)
	$$(call build_cpp_object)

$$(LOCAL_OUT_PATH)/%.o: $$(LOCAL_PATH)/%.cxx | $$(LOCAL_OUT_PATH)
	$$(Q)$$(MKDIR) $$(@D)
	$$(call build_cpp_object)

$$(LOCAL_OUT_PATH):
	$$(Q)$$(MKDIR) $$@

MODULES := $$(MODULES) $$(LOCAL_MODULE_PATH)

-include $$(LOCAL_OBJECT:%.o=%.d)
endef

define build_c_object
@echo "[CC]    $< => $@"
$(Q)$(CC) -o $@ $(CFLAGS) -MD $(1) -c $<
endef

define build_cpp_object
@echo "[C++]   $< => $@"
$(Q)$(CPP) -o $@ $(CPPFLAGS) -MD $(1) -c $<
endef

define link_c_execute
@echo "[LD]    $@ <= $(notdir $^)"
$(Q)$(LD) -o $@ $^ $(LDFLAGS)
$(call strip_files,$@)
endef

define link_c_execute_action
ifeq ($$(filter static release,$$(BUILD_TYPE)),)
LOCAL_DEPEND := $$(foreach lib,$$(LOCAL_LIBRARY),$$(OUT_LIB)/$$(lib).so)
$$(LOCAL_MODULE_PATH): LDFLAGS := $$(patsubst lib%,-l%,$$(LOCAL_LIBRARY)) $$(LDFLAGS)
$$(LOCAL_MODULE_PATH): $$(LOCAL_OBJECT) | $$(LOCAL_DEPEND)
	$$(call link_c_execute)
else
LOCAL_DEPEND := $$(foreach lib,$$(LOCAL_LIBRARY),$$(OUT_LIB)/$$(lib).a)
$$(LOCAL_MODULE_PATH): $$(LOCAL_OBJECT) $$(LOCAL_DEPEND)
	$$(call link_c_execute)
endif
endef

define link_c_libso
@echo "[LD]    $@ <= $(notdir $^)"
$(Q)$(LD) -shared -o $@ $^ $(LDFLAGS)
$(call strip_files,$@)
endef

define link_c_liba
@echo "[AR]    $@ <= $(notdir $^)"
$(Q)$(AR) cur $@ $^
endef

define search_all_files
$(patsubst $(LOCAL_PATH)/%,%,$(wildcard $(addprefix $(LOCAL_PATH)/,$(1))))
endef

define strip_files
$(Q)[ "$(BUILD_TYPE)" = "debug" ] || $(STRIP) -s $(1)
endef

define build_as_execute
LOCAL_SOURCE := $(1)
LOCAL_MODULE := $(basename $(notdir $(1)))
include $(BUILD_EXECUTE)
endef
