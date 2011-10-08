all:
	sed 's/^\s*#ifdef\s\+\(HAVE_SCANDIR\)/#ifndef \1/g' preload/interface.master -i
	./autogen.sh
	make  prefix=$(UTILS_PATH)/usr install
