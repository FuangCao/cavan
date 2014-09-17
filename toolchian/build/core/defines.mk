define pr_red_info
echo -e "\033[31m$1\033[0m"
endef

define download_package_simple
{ \
	file_list="$$(find $(wildcard $(PACKAGE_PATH)/ $(DOWNLOAD_PATH)/) -type f -and -name $1 $(foreach type,$(PACKAGE_TYPES),-or -name $1.$(type)))"; \
	[ "$${file_list}" ] || \
	{ \
		cd $(DOWNLOAD_PATH) || \
		{ \
			$(call pr_red_info,Entry to directory $(DOWNLOAD_PATH) failed); \
			exit 1; \
		}; \
		for url in $2; \
		do \
			case "$${url}" in \
				*.rar | *.zip | *.bz2 | *.gz | *.xz) \
					file_list="$$(basename $${url})"; \
					if test -f "$(PACKAGE_PATH)/$${file_list}"; \
					then \
						file_list="$(PACKAGE_PATH)/$${file_list}"; \
					else \
						test -f "$${file_list}" || $(DOWNLOAD_COMMAND) $${url} || exit 1; \
						file_list="$(DOWNLOAD_PATH)/$${file_list}"; \
					fi; \
					;; \
				*) \
					if [ "$3" ]; \
					then \
						type_list="$3"; \
					else \
						type_list="$(DOWNLOAD_TYPES)"; \
					fi; \
					for type in $${type_list}; \
					do \
						file_list="$1.$${type}"; \
						$(DOWNLOAD_COMMAND) "$${url}/$${file_list}" && \
						{ \
							file_list="$(DOWNLOAD_PATH)/$${file_list}"; \
							break; \
						}; \
						rm $${file_list} -rf; \
					done; \
					;; \
			esac; \
			[ -f "$${file_list}" ] && break; \
		done; \
		[ -f "$${file_list}" ] || \
		{ \
			$(call pr_red_info,Download $1 from $2 failed); \
			exit 1; \
		}; \
	}; \
}
endef

define simple_decompression_file
{ \
	temp_decomp="$(DECOMP_PATH)/$1"; \
	$(call download_package_simple,$1,$3,$4); \
	for pkg in $${file_list}; \
	do \
		echo "Decompression $${pkg} => $${temp_decomp}"; \
		rm $${temp_decomp} -rf && mkdir $${temp_decomp} -pv && cd $${temp_decomp} || continue; \
		while [ -f "$${pkg}" ]; \
		do \
			case "$${pkg}" in \
				*.tar.bz2 | *.tbz) tar --use-compress-program bzip2 -xf $${pkg} || tar -xf $${pkg} || break;; \
				*.tar.gz | *.tgz) tar --use-compress-program gzip -xf $${pkg} || tar -xf $${pkg} || break;; \
				*.tar.xz | *.txz) tar --use-compress-program xz -xf $${pkg} || tar -xf $${pkg} || break;; \
				*.zip | *.apk) unzip -o $${pkg} || break;; \
				*.rar) rar x -o+ $${pkg} || break;; \
				*) tar -xf $${pkg} || break;; \
			esac; \
			for pkg in *; \
			do \
				[ -d "$${pkg}" ] && break; \
			done; \
		done; \
		test -d "$${pkg}" && rm $2 -rf && mv $${pkg} $2 -v && break; \
		rm $${temp_decomp} -rf; \
	done; \
}
endef

define apply_patchs
{ \
	cd $2 && \
	for fn in $(wildcard $(PATCH_PATH)/$1*.patch); \
	do \
		echo "Apply patch $${fn} => $2"; \
		patch -p1 -i $${fn}; \
	done; \
}
endef

define decompression_file
$(eval pkg_name = $(notdir $1))
if [ ! -d "$1" -o "$(FORCE_DECOMPRESSION)" = "yes" ]; \
then \
	$(call simple_decompression_file,$(pkg_name),$1,$2,$3); \
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
$(Q)mkdir $1 -pv
endef

define install_application
$(eval export PACKAGE_NAME = $(notdir $@))
$(eval export PACKAGE_BASENAME = $(firstword $(subst -, ,$(PACKAGE_NAME))))
$(eval export PACKAGE_SOURCE = $(SRC_PATH)/$(PACKAGE_NAME))
rm $(PACKAGE_SOURCE) -rf
$(call decompression_file,$(PACKAGE_SOURCE),$1,$2)
test -f "$(PACKAGE_SOURCE)/configure" && sed 's#^\s*(./conftest\s*$$#(#g' $(PACKAGE_SOURCE)/configure -i || echo "No configure script"
$(eval makefile-path = $(firstword $(wildcard $3/$(PACKAGE_NAME).mk $3/$(PACKAGE_BASENAME)*.mk)))
+if test -n "$(makefile-path)"; \
then \
	make -C $(PACKAGE_SOURCE) -f $(makefile-path); \
else \
	cd $(PACKAGE_SOURCE) && ($4); \
fi
$(call generate_mark)
endef

define execute_retry
for i in 1 2 3; \
do \
	($1) && break; \
done
endef

define install_utils
$(call install_application,$2,$3,$(BUILD_UTILS),./configure $(UTILS_COMMON_CONFIG) $1 && $(call execute_retry,make && make install))
endef

define install_library
$(call install_application,$2,$3,$(BUILD_TOOLCHIAN),./configure $(LIBRARY_COMMON_CONFIG) $1 && $(call execute_retry,make && make DESTDIR="$(ROOTFS_PATH)" install))
endef

define install_rootfs
$(call install_application,$2,$3,$(BUILD_ROOTFS),CFLAGS="-fPIC" sb2 ./configure $1 && sb2 make && { sb2 -m install make install || sb2 make DESTDIR="$(ROOTFS_PATH)" install; })
endef

define download_package
$(call download_package_simple,$1,$2,$3)
$(call generate_mark)
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

define generate_makefile
test "$3" -nt "$2" -a "$3" -nt "$(PYTHON_PARSER)" || python "$(PYTHON_PARSER)" "$1" "$2" "$3"
endef
