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

define install_application
$(eval app-name = $(notdir $@))
$(eval app-basename = $(firstword $(subst -, ,$(app-name))))
$(eval src-path = $(SRC_PATH)/$(app-name))
rm $(src-path) -rf
$(call decompression_file,$(src-path),$1)
$(eval makefile-path = $(firstword $(wildcard $2/$(app-name).mk $2/$(app-basename).mk)))
+if test -n "$(makefile-path)"; \
then \
	make -C $(src-path) -f $(makefile-path); \
else \
	cd $(src-path) && $3; \
fi
$(call generate_mark)
endef

define install_utils
$(call install_application,$2,$(BUILD_UTILS),./configure $1 && make && make install)
endef

define install_rootfs
$(call install_application,$2,$(BUILD_ROOTFS),sb2 ./configure $1 && sb2 make && sb2 make DESTDIR="$(ROOTFS_PATH)" install)
endef

define copy_shared_library
cp $1/*.so* $2 -av
endef
