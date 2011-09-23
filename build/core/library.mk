all: $(TARGET_LIBA) $(TARGET_LIBSO)

$(TARGET_LIBA): $(TARGET_OBJ)
	$(call build_liba_file)

$(TARGET_LIBSO): $(TARGET_OBJ)
	$(call build_libso_file)

$(TARGET_OBJ): $(LIB_OBJECT)
	$(call build_libo_file)

include $(LIB_DEPEND)
include $(DEFINES_MAKEFILE)
