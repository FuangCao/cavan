$(info TARGET_CAVAN_ELF = $(TARGET_CAVAN_ELF))

all: $(TARGET_CAVAN_ELF)

$(TARGET_CAVAN_ELF): $(TARGET_CAVAN_OBJ)
	@echo "$^ => $@"
	$(call build_elf_file)

$(TARGET_CAVAN_OBJ): $(CAVAN_OBJECT) $(APP_CORE_OBJECT)
	$(call build_libo_file)

$(APP_CORE_SOURCE): $(CAVAN_CMD) $(CAVAN_CMD)
	$(call touch_file)

include $(CAVAN_DEPEND)
include $(CAVAN_SOURCE_DEPEND)
include $(APP_CORE_DEPEND)
include $(DEFINES_MAKEFILE)
