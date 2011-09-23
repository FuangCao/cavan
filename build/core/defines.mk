define build_object_file
@echo "[CC] $^ => $@"
$(Q)$(CC) $(CFLAGS) -o $@ -c $^
endef

define build_elf_file
@echo "[CC] $^ => $@"
$(Q)$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)
endef

define build_liba_file
@echo "[AR] $^ => $@"
$(Q)$(AR) cur $@ $^
endef

define build_libso_file
@echo "[CC] $^ => $@"
$(Q)$(CC) -fPIC -shared -o $@ $^
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

define build_obj_depend
@echo "Automatically Generate $@"
$(eval obj-dir = $(dir $@))
@for action in $(foreach fn,$^,$(basename $(obj-dir)$(notdir $(fn))).o:$(fn)); \
do \
	echo $${action}; \
	echo '	$$(call build_object_file)'; \
	echo; \
done > $@
endef

define build_elf_depend
@echo "Automatically Generate $@"
$(eval elf-dir = $(dir $@))
$(if $2,$(eval elf-dir := $(elf-dir)$2-))
@for action in $(foreach fn,$(basename $(notdir $^)),$(elf-dir)$(fn):$1/$(fn).o); \
do \
	echo $${action}; \
	echo '	$$(call build_elf_file)'; \
	echo; \
done > $@
endef

define app_to_cavan_source
@echo "Automatically Generate $@"
$(eval cavan-main = do_cavan_$(basename $(notdir $@)))
@sed	-e 's/^\s*int\s\+main\s*(\(.*\))\s*$$/int $(cavan-main)(\1)/g' \
		-e	's/^\s*void\s\+main\s*(\(.*\))\s*$$/int $(cavan-main)(\1)/g' \
		$(filter %/$(notdir $@),$^) > $@
endef

define build_cavan_source_depend
@echo "Automatically Generate $@"
$(eval cavan-dir = $(dir $@))
@for action in $(foreach fn,$^,$(cavan-dir)$(notdir $(fn)):$(fn)); \
do \
	echo $${action}; \
	echo '	$$(call app_to_cavan_source)'; \
	echo; \
done > $@
endef

define build_cavan_depend
@echo "Automatically Generate $@"
@: > $1
@echo "#pragma once" > $2
@echo >> $2
$(eval cavan-dir = $(dir $@))
@for fn in $(basename $(notdir $^)); \
do \
	echo $(cavan-dir)$${fn}.o:$(cavan-dir)$${fn}.c; \
	echo '	$$(call build_object_file)'; \
	echo; \
	echo "{\"$${fn}\", do_cavan_$${fn}}," >> $1; \
	echo "int do_cavan_$${fn}(int argc, char *argv[]);" >> $2; \
done > $@
endef
