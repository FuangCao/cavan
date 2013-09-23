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
		echo "{\"$$$${obj}\", do_cavan_$$$${obj}},"; \
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
endef

define module_common_action
ifeq ($$(LOCAL_MODULE),)
$$(error Please give module name for $$(LOCAL_PATH))
endif

LOCAL_OUT_PATH := $$(OUT_OBJ)/$$(LOCAL_MODULE)_$(1)
LOCAL_OBJECT := $$(patsubst %,$$(LOCAL_OUT_PATH)/%.o,$$(basename $$(LOCAL_SOURCE)))
LOCAL_DEPEND := $$(LOCAL_OBJECT:%.o=%.d)
LOCAL_CFLAGS += $$(addprefix -I,$$(LOCAL_INCLUDE))

ifeq ($$(filter %.cpp %.cxx %.cc,$$(LOCAL_SOURCE)),)
$$(LOCAL_OBJECT) $$(LOCAL_DEPEND) $$(LOCAL_MODULE_PATH): CFLAGS += $$(LOCAL_CFLAGS)
else
$$(LOCAL_OBJECT) $$(LOCAL_DEPEND) $$(LOCAL_MODULE_PATH): CC = $$(CPP)
$$(LOCAL_OBJECT) $$(LOCAL_DEPEND) $$(LOCAL_MODULE_PATH): CFLAGS = $$(CPPFLAGS) $$(LOCAL_CFLAGS)
endif

$$(if $$(filter package,$(1)),$$(eval $$(call module_package_action)))

$$(LOCAL_OUT_PATH)/%.o: | $$(LOCAL_OUT_PATH)/%.d
	$$(call build_c_object)

$$(LOCAL_OUT_PATH)/%.d: $$(LOCAL_PATH)/%.c | $$(LOCAL_OUT_PATH)
	$$(call gen_depend_file)

$$(LOCAL_OUT_PATH)/%.d: $$(LOCAL_PATH)/%.cc | $$(LOCAL_OUT_PATH)
	$$(call gen_depend_file)

$$(LOCAL_OUT_PATH)/%.d: $$(LOCAL_PATH)/%.cpp | $$(LOCAL_OUT_PATH)
	$$(call gen_depend_file)

$$(LOCAL_OUT_PATH)/%.d: $$(LOCAL_PATH)/%.cxx | $$(LOCAL_OUT_PATH)
	$$(call gen_depend_file)

$$(LOCAL_OUT_PATH):
	$$(Q)$$(MKDIR) $$@

-include $$(LOCAL_DEPEND)
MODULES := $$(MODULES) $$(LOCAL_MODULE_PATH)
endef

define gen_depend_file
@echo "[GEN]   $< => $@"
$(Q)$(CC) -M $(CFLAGS) -MT $(patsubst %.d,%.o,$@) -MF $@ $<
endef

define build_c_object
@echo "[CC]    $< => $@"
$(Q)$(CC) -o $@ $(CFLAGS) $(1) -c $<
endef

define link_c_execute
ifeq ($$(findstring -static,$$(LDFLAGS)),)
LOCAL_DEPEND_LIB := $$(foreach lib,$$(LOCAL_LIBRARY),$$(OUT_LIB)/$$(lib).so)
$$(LOCAL_MODULE_PATH): LDFLAGS := $$(patsubst lib%,-l%,$$(LOCAL_LIBRARY)) $$(LDFLAGS)
$$(LOCAL_MODULE_PATH): $$(LOCAL_OBJECT) | $$(LOCAL_DEPEND_LIB)
else
LOCAL_DEPEND_LIB := $$(foreach lib,$$(LOCAL_LIBRARY),$$(OUT_LIB)/$$(lib).a)
$$(LOCAL_MODULE_PATH): $$(LOCAL_OBJECT) $$(LOCAL_DEPEND_LIB)
endif
	@echo "[LD]    $$@ <= $$(notdir $$^)"
	$$(Q)$$(CC) -o $$@ $$^ $$(LDFLAGS)
	$$(call strip_files,$$@)
endef

define link_c_libso
@echo "[LD]    $@ <= $(notdir $^)"
$(Q)$(CC) -shared -o $@ $^ $(LDFLAGS)
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
$(Q)$(STRIP) -s $(1)
endef

define build_as_execute
LOCAL_SOURCE := $(1)
LOCAL_MODULE := $(basename $(notdir $(1)))
include $(BUILD_EXECUTE)
endef
