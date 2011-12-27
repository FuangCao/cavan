define filter_out_lastwords
$(wordlist 1,$(shell expr $(words $(2)) - $(1)),$(2))
endef

define file_path_convert
$(patsubst %.c,$(2)%$(3),$(notdir $(1)))
endef

define compile_file
@echo "[CC]\t$(2) => $(1)"
$(Q)$(CC) -c $(CFLAGS) $(3) -o $(1) $(2)
endef

define strip_file
$(Q)$(STRIP) $(1)
endef

define link_excuteable
@echo "[LD]\t$(1) <= $(2)"
$(Q)$(CC) -o $(1) $(2) $(APP_LDFLAGS) $(LDFLAGS)
endef

define link_static_library
@echo "[AR]\t$(1) <= $(2)"
$(Q)$(AR) cur $(1) $(2)
$(call strip_file,$(1))
endef

define link_shared_library
@echo "[LD]\t$(1) <= $(2)"
$(Q)$(CC) -shared -o $(1) $(2)
$(call strip_file,$(1))
endef

define link_object_file
@echo "[LD]\t$(1) <= $(2)"
$(Q)$(LD) -o $(1) -r $(2)
endef

define load_cavan_make
$(eval source-app =)
$(eval source-lib =)
$(eval include $(1)/$(MAKEFILE_CAVAN))
$(eval APP_SRC_FILES += $(addprefix $(1)/,$(source-app)))
$(eval LIB_SRC_FILES += $(addprefix $(1)/,$(source-lib)))
endef

define find_source_files
$(if $(wildcard $(1)/$(MAKEFILE_CAVAN)),$(call load_cavan_make,$(1)),$(eval $(2) += $(wildcard $(1)/*.c)))
endef

define build_app_action
$(call find_source_files,$(1),APP_SRC_FILES)
$$(OUT_APP)/%.o: $(1)/%.c
	$$(call compile_file,$$@,$$^)
$$(OUT_CAVAN)/%.c: $(1)/%.c
	@echo "[GEN]\t$$< => $$@"
	$$(eval main-name = do_cavan_$$*)
	@sed	-e "s/^\s*int\s\+main\s*\((.*)\)/int $$(main-name)\1/g" \
			-e "s/^\s*void\s\+main\s*\((.*)\)/void $$(main-name)\1/g" \
			$$< > $$@
endef

define build_lib_action
$(call find_source_files,$(1),LIB_SRC_FILES)
$$(OUT_LIB)/%.o: $(1)/%.c
	$$(call compile_file,$$@,$$^,-fPIC)
endef
