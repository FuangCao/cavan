LOCAL_LIBRARY := libcavan

$(foreach fn,$(call search_all_files,*.c),$(eval $(call build_as_execute,$(fn))))
