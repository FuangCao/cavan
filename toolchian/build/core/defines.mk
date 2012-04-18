define simple_decompression_file
temp_decomp="$(DECOMP_PATH)/$1"; \
file_list="$(strip $(foreach type,${PACKAGE_TYPES},$(wildcard $(PACKAGE_PATH)/$1.*$(type) $(DOWNLOAD_PATH)/$1.*$(type))))"; \
[ -n "$${file_list}" ] || \
{ \
	cd $(DOWNLOAD_PATH); \
	case "$3" in \
		*.rar | *.zip | *.bz2 | *.gz | *.xz) \
			file_list="$(notdir $3)"; \
			if test -f "$(PACKAGE_PATH)/$${file_list}"; \
			then \
				file_list="$(PACKAGE_PATH)/$${file_list}"; \
			else \
				test -f "$${file_list}" || $(DOWNLOAD_COMMAND) $3; \
				file_list="$(DOWNLOAD_PATH)/$${file_list}"; \
			fi; \
			;; \
		*) \
			for type in $(DOWNLOAD_TYPES); \
			do \
				file_list="$1.$${type}"; \
				$(DOWNLOAD_COMMAND) "$3/$${file_list}" && \
				{ \
					file_list="$(DOWNLOAD_PATH)/$${file_list}"; \
					break; \
				}; \
				rm $${file_list} -rf; \
			done; \
			;; \
	esac; \
}; \
for pkg in $${file_list}; \
do \
	echo "Decompression $${pkg} => $${temp_decomp}"; \
	rm $${temp_decomp} -rf && mkdir $${temp_decomp} -pv && cd $${temp_decomp} || continue; \
	while test -f $${pkg}; \
	do \
		case "$${pkg}" in \
			*.tar.bz2) tar --use-compress-program bzip2 -xf $${pkg};; \
			*.tar.gz) tar --use-compress-program gzip -xf $${pkg};; \
			*.tar.xz) tar --use-compress-program xz -xf $${pkg};; \
			*.zip) unzip -o $${pkg};; \
			*.rar) rar x -o+ $${pkg};; \
			*) tar -xf $${pkg};; \
		esac; \
		for pkg in *; \
		do \
			[ -d "$${pkg}" ] && break; \
		done; \
	done; \
	test -d "$${pkg}" && rm $2 -rf && mv $${pkg} $2 -v && break; \
	rm $${temp_decomp} -rf; \
done
endef

define apply_patchs
cd $2 && \
for fn in $(wildcard $(PATCH_PATH)/$1*.patch); \
do \
	echo "Apply patch $${fn} => $2"; \
	patch -Np1 -i $${fn}; \
done
endef

define decompression_file
$(eval pkg_name = $(notdir $1))
if [ ! -d "$1" -o "$(FORCE_DECOMPRESSION)" = "yes" ]; \
then \
	$(call simple_decompression_file,$(pkg_name),$1,$2); \
	$(call apply_patchs,$(pkg_name),$1); \
fi
endef

define find_source_package
$(wildcard $(PACKAGE_PATH)/$1*)
endef

define generate_mark
$(Q)echo "Generate Mark $@"
$(Q)echo "$@" > $@
endef

define print_action
$(Q)echo "Action \"$@\""
endef

define remake_directory
$(Q)echo "Remake Directory $1"
$(Q)rm $1 -rf
$(Q)mkdir $1 -pv
endef

define install_application
$(eval export PACKAGE_NAME = $(notdir $@))
$(eval export PACKAGE_BASENAME = $(firstword $(subst -, ,$(PACKAGE_NAME))))
$(eval export PACKAGE_SOURCE = $(SRC_PATH)/$(PACKAGE_NAME))
rm $(PACKAGE_SOURCE) -rf
$(call decompression_file,$(PACKAGE_SOURCE),$1)
test -f "$(PACKAGE_SOURCE)/configure" && sed 's#^\s*(./conftest\s*$$#(#g' $(PACKAGE_SOURCE)/configure -i || echo "No configure script"
$(eval makefile-path = $(firstword $(wildcard $2/$(PACKAGE_NAME).mk $2/$(PACKAGE_BASENAME)*.mk)))
+if test -n "$(makefile-path)"; \
then \
	make -C $(PACKAGE_SOURCE) -f $(makefile-path); \
else \
	cd $(PACKAGE_SOURCE) && $3; \
fi
$(call generate_mark)
endef

define install_utils
$(call install_application,$2,$(BUILD_UTILS),./configure $(UTILS_COMMON_CONFIG) $1 && make && make install)
endef

define install_library
$(call install_application,$2,$(BUILD_TOOLCHIAN),./configure $(LIBRARY_COMMON_CONFIG) $1 && make && make DESTDIR="$(ROOTFS_PATH)" install)
endef

define install_rootfs
$(call install_application,$2,$(BUILD_ROOTFS),CFLAGS="-fPIC" sb2 ./configure $1 && sb2 make && sb2 -m install make install)
endef

define auto_make
$(Q)python $(PYTHON_PARSER) -f $1 -m $2 -o $3 $4
$(Q)+make -f $(MAKEFILE_DEFINES) -f $3/name.mk -f $3/depend.mk
$(call generate_mark)
endef

define remove_files
$(Q)for fn in $1; \
do \
	echo -n "Remove $${fn} ... "; \
	rm $${fn} -rf; \
	echo "OK"; \
done
endef

define remake_device_node
$(Q)echo "Remake device node $1"
$(Q)rm $1 -rf
$(Q)sudo mknod $1 $2 $3 $4
endef

define find_makefile
$(word 1,$(wildcard $1))
endef
