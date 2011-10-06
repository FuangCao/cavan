define simple_decompression_file
temp_decomp="$(DECOMP_PATH)/$1"; \
file_list="$(wildcard $(PACKAGE_PATH)/$1.tar.* $(DOWNLOAD_PATH)/$1.tar.*)"; \
[ -n "$${file_list}" ] || \
{ \
	cd $(DOWNLOAD_PATH); \
	case "$3" in \
		*.tar.*) \
			file_list=$(notdir $3); \
			test -f $${file_list} || wget $3; \
			;; \
		*) \
			for type in $(DOWNLOAD_TYPES); \
			do \
				file_list="$1.$${type}"; \
				wget "$3/$${file_list}" && break; \
				rm $${file_list} -rf; \
			done; \
			;; \
	esac; \
}; \
for pkg in $${file_list}; \
do \
	rm $${temp_decomp} -rf; \
	mkdir $${temp_decomp} -pv; \
	echo "Decompression $${pkg} => $${temp_decomp}"; \
	tar -xf $${pkg} -C $${temp_decomp} && break; \
done; \
rm $2 -rf; \
mv $${temp_decomp}/* $2; \
rm $${temp_decomp} -rf
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
if [ ! -d "$1" -o "$(FORCE_DECOMPRESSION)" = "force" ]; \
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

define install_utils
$(eval app-name = $(notdir $@))
$(eval src-path = $(SRC_PATH)/$(app-name))
rm $(src-path) -rf
$(call decompression_file,$(src-path),$2)
$(eval makefile-path = $(BUILD_UTILS)/$(app-name).mk)
if test -f $(makefile-path); \
then \
	make -C $(src-path) -f $(makefile-path); \
else \
	cd $(src-path) && \
	./configure $1 && \
	make -j4 && \
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
if test -f $(makefile-path); \
then \
	make -C $(src-path) -f $(makefile-path); \
else \
	cd $(src-path) && \
	./configure $1 --build=$(CAVAN_BUILD_PLAT) --host=$(CAVAN_TARGET_PLAT) --target=$(CAVAN_TARGET_PLAT) && \
	make -j4 && \
	make DESTDIR="$(SYSROOT_PATH)" install; \
fi
$(call generate_mark)
endef
