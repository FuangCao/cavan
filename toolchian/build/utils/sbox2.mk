all:
	$(Q)sed 's/^\s*#ifdef\s\+\(HAVE_SCANDIR\)/#ifndef \1/g' preload/interface.master -i
	$(Q)./autogen.sh
	$(Q)+make  prefix=$(UTILS_PATH)/usr install
