include $(MAKEFILE_DEFINES)

all:
	./configure $(ROOTFS_COMMON_CONFIG) --prefix=/usr
	sed 's/\s*CC\s*=.*$$/CC = $(CAVAN_TARGET_PLAT)-gcc/g' $$(find -name [Mm]akefile) -i
	make -j4
	$(call install_to_rootfs)
