define simple_decompression_file
temp_decomp="$(DECOMP_PATH)/$1"; \
file_list="$(wildcard $(PACKAGE_PATH)/$1*.tar.*)"; \
[ -n "$${file_list}" ] || \
{ \
	cd $(DOWNLOAD_PATH); \
	for type in $(DOWNLOAD_TYPES); \
	do \
		file_list="$1.$${type}"; \
		[ -f "$${file_list}" ] || wget "$3/$${file_list}" && break; \
		rm $${file_list} -rf; \
	done; \
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

define install_application
$(eval app-name = $(notdir $@))
$(eval src-path = $(SRC_UTILS)/$(app-name))
$(call decompression_file,$(src-path),$2)
$(eval out-path = $(OUT_UTILS)/$(app-name))
$(call remake_directory,$(out-path))
cd $(out-path) && $(src-path)/configure $1
+make -C $(out-path)
+make -C $(out-path) DESTDIR=$(UTILS_PATH) install
$(call generate_mark)
endef
