all: $(ELF_OBJECT)

include $(ELF_DEPEND)
include $(APP_DEPEND)
include $(DEFINES_MAKEFILE)
