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
@echo [LD] "$^ => $@"
$(Q)$(LD) -o $@ -r $^
endef

define build_obj_depend
$(eval obj-dir = $(dir $@))
@for action in $(foreach fn,$^,$(basename $(obj-dir)$(notdir $(fn))).o:$(fn)); \
do \
	echo $${action}; \
	echo '	$$(call build_object_file)'; \
	echo; \
done > $@
endef

define build_elf_depend
$(eval elf-dir = $(dir $@))
$(if $2,$(eval elf-dir := $(elf-dir)$2-))
@for action in $(foreach fn,$(basename $(notdir $^)),$(elf-dir)$(fn):$1/$(fn).o); \
do \
	echo $${action}; \
	echo '	$$(call build_elf_file)'; \
	echo; \
done > $@
endef
