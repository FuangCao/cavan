LOCAL_LIBRARY := libcavan libcavan++

$(foreach fn,$(call search_all_files,*.c *.cc *.cpp *.cxx),$(eval $(call build_as_execute,$(fn))))
