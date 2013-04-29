define build_object_file
@echo "[CC] $^ => $@"
$(Q)$(CC) $(CFLAGS) -o $@ -c $^
endef

define build_elf_file
@echo "[LD] $^ => $@"
$(Q)$(CC) -o $@ $^ $(LDFLAGS)
endef

define build_liba_file
@echo "[AR] $^ => $@"
$(Q)$(AR) cur $@ $^
endef

define build_libso_file
@echo "[LD] $^ => $@"
$(Q)$(CC) -shared -o $@ $^ $(LDFLAGS)
endef

define build_libo_file
@echo "[LD] $^ => $@"
$(Q)$(LD) -o $@ -r $^
endef

define remove_file
@echo "[RM] $1"
@rm $1 -rf
endef

define touch_file
@echo "[TC] $@"
@touch $@
endef

define make_directory
@echo "[MKDIR] $@"
@mkdir $@ -p
endef

define source_to_object
$(patsubst %.c,$1/%.o,$(notdir $2))
endef

define generate_cavan_source
@echo "Automatically Generate $@"
$(eval cavan-main = $(CAVAN_MAIN_FUNC_PREFIX)$(basename $(notdir $@)))
@sed	-e 's/^\s*int\s\+main\s*(\(.*\))\s*$$/int $(cavan-main)(\1)/g' \
		-e	's/^\s*void\s\+main\s*(\(.*\))\s*$$/int $(cavan-main)(\1)/g' \
		$(filter %/$(notdir $@),$^) > $@
endef

define generate_obj_depend
@echo "Automatically Generate $1"
$(eval obj-dir = $(dir $1))
@for action in $(foreach fn,$2,$(basename $(obj-dir)$(notdir $(fn))).o:$(fn)); \
do \
	echo $${action}; \
	echo '	$$(call build_object_file)'; \
	echo; \
done > $1
endef

define generate_elf_depend
@echo "Automatically Generate $1"
$(eval elf-dir = $(dir $1))
$(if $(ELF_PREFIX),$(eval elf-dir := $(elf-dir)$(ELF_PREFIX)-))
@for action in $(foreach fn,$(basename $(notdir $3)),$(elf-dir)$(fn):$2/$(fn).o); \
do \
	echo $${action}; \
	echo '	$$(call build_elf_file)'; \
	echo; \
done > $1
endef

define generate_src_depend
@echo "Automatically Generate $1"
$(eval cavan-dir = $(dir $1))
@for action in $(foreach fn,$2,$(cavan-dir)$(notdir $(fn)):$(fn)); \
do \
	echo $${action}; \
	echo '	$$(call generate_cavan_source)'; \
	echo; \
done > $1
endef

define generate_cavan_obj_depend
@echo "Automatically Generate $1"
$(eval obj-dir = $(dir $1))
@for action in $(foreach fn,$2,$(basename $(obj-dir)$(notdir $(fn))).o:$(fn)); \
do \
	echo $${action}; \
	echo '	$$(call build_object_file)'; \
	echo; \
done > $1
endef

define generate_map_source
@echo "Automatically Generate $@"
@for fn in $(basename $(notdir $^)); \
do \
	echo "{\"$${fn}\", $(CAVAN_MAIN_FUNC_PREFIX)$${fn}},"; \
done > $@
endef

define generate_map_header
@echo "Automatically Generate $@"
@echo '#pragma once' > $@
@echo >> $@
@for fn in $(basename $(notdir $^)); \
do \
	echo "int $(CAVAN_MAIN_FUNC_PREFIX)$${fn}(int argc, char *argv[]);"; \
done >> $@
endef

define write_config
@{ \
	echo "ARCH = $(ARCH)"; \
	echo "CROSS_COMPILE = $(CROSS_COMPILE)"; \
	echo "BUILD_TYPE = $(BUILD_TYPE)"; \
	echo "APP = $(APP)"; \
	echo "LIB = $(LIB)"; \
} > $1
endef
