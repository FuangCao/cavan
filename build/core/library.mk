include $(DEFINES_MAKEFILE)

CFLAGS += -fPIC
CPPFLAGS += -fPIC

LIB_OBJECT = $(call source_to_object,$(OUT_LIB),$(LIB_SOURCE))

all: $(TARGET_LIBA) $(TARGET_LIBSO)

$(TARGET_LIBA): $(TARGET_LIB_OBJ)
	$(call build_liba_file)

$(TARGET_LIBSO): $(TARGET_LIB_OBJ)
	$(call build_libso_file)

$(TARGET_LIB_OBJ): $(LIB_OBJECT)
	$(call build_libo_file)

include $(LIB_DEPEND)
