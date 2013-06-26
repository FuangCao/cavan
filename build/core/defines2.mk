define filter_out_lastwords
$(wordlist 1,$(shell expr $(words $(2)) - $(1)),$(2))
endef

define file_path_convert
$(patsubst %.c,$(2)%$(3),$(notdir $(1)))
endef

define compile_file
@echo "[CC]    $(2) => $(1)"
$(Q)$(CC) -c $(CFLAGS) $(3) -o $(1) $(2)
endef

define strip_file
$(Q)$(STRIP) -s $(1)
endef

define link_excuteable
@echo "[LD]    $(1) <= $(2)"
$(Q)$(CC) -o $(1) $(2) $(APP_LDFLAGS) $(LDFLAGS)
$(call strip_file,$(1))
endef

define link_static_library
@echo "[AR]    $(1) <= $(2)"
$(Q)$(AR) cur $(1) $(2)
endef

define link_shared_library
@echo "[LD]    $(1) <= $(2)"
$(Q)$(CC) -shared -o $(1) $(2) $(LDFLAGS)
$(call strip_file,$(1))
endef

define link_object_file
@echo "[LD]    $(1) <= $(2)"
$(Q)$(LD) -o $(1) -r $(2)
endef

define find_source_files
$(eval $(2) += $(wildcard $(1)/*.c))
endef

define build_app_action
$(call find_source_files,$(1),APP_SRC_FILES)
$$(OUT_APP)/%.o: $(1)/%.c
	$$(call compile_file,$$@,$$^)
$$(OUT_CAVAN)/%.o: $(1)/%.c
	$$(call compile_file,$$@,$$^,-Dmain=do_cavan_$$*)
endef

define build_lib_action
$(call find_source_files,$(1),LIB_SRC_FILES)
$$(OUT_LIB)/%.o: $(1)/%.c
	$$(call compile_file,$$@,$$^,-fPIC -Wmissing-prototypes -Wmissing-declarations)
endef
