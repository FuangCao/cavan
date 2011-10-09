define decompression_all_file
endef

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
echo "Generate Mark $@"
echo "$@" > $@
endef

define print_action
echo "Action \"$@\""
endef

define remake_directory
echo "Remake Directory $1"
rm $1 -rf
mkdir $1 -pv
endef

define install_to_sysroot
@echo "Install sysroot package"
make DESTDIR="$(SYSROOT_PATH)" install
endef

define install_to_rootfs
@echo "Install rootfs package"
make DESTDIR="$(ROOTFS_PATH)" install
endef

define install_to_emulator
@echo "Install emulator package"
make DESTDIR="$(EMULATOR_PATH)" install
endef

define install_utils
$(eval app-name = $(notdir $@))
$(eval src-path = $(SRC_PATH)/$(app-name))
rm $(src-path) -rf
$(call decompression_file,$(src-path),$2)
$(eval makefile-path = $(BUILD_UTILS)/$(app-name).mk)
+if test -f $(makefile-path); \
then \
	make -C $(src-path) -f $(makefile-path); \
else \
	cd $(src-path) && \
	./configure $1 && \
	make && \
	make install; \
fi
$(call generate_mark)
endef

define install_library
$(eval app-name = $(notdir $@))
$(eval src-path = $(SRC_PATH)/$(app-name))
rm $(src-path) -rf
$(call decompression_file,$(src-path),$2)
$(eval makefile-path = $(BUILD_LIBRARY)/$(app-name).mk)
+if test -f $(makefile-path); \
then \
	make -C $(src-path) -f $(makefile-path); \
else \
	cd $(src-path) && \
	./configure $(LIBRARY_COMMON_CONFIG) $1 && \
	make && \
	make DESTDIR="$(SYSROOT_PATH)" install; \
fi
$(call generate_mark)
endef

define install_rootfs
$(eval app-name = $(notdir $@))
$(eval src-path = $(SRC_PATH)/$(app-name))
rm $(src-path) -rf
$(call decompression_file,$(src-path),$2)
$(eval makefile-path = $(BUILD_ROOTFS)/$(app-name).mk)
+if test -f $(makefile-path); \
then \
	make -C $(src-path) -f $(makefile-path); \
else \
	cd $(src-path) && \
	./configure $(ROOTFS_COMMON_CONFIG) $1 && \
	make && \
	make DESTDIR="$(ROOTFS_PATH)" install; \
fi
$(call generate_mark)
endef
