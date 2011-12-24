ROOT_PATH = $(PWD)
CAVAN_NAME = cavan
APP_PATH = app
LIB_PATH = lib
INCLUDE_PATH = $(ROOT_PATH)/include
APP_CORE_PATH = $(APP_PATH)/core

OUT_PATH = out/$(ARCH)/$(BUILD_TYPE)
OUT_LIB = $(OUT_PATH)/lib
OUT_APP = $(OUT_PATH)/app
OUT_BIN = $(OUT_PATH)/bin
OUT_CAVAN = $(OUT_PATH)/cavan

CC = $(Q)echo "[CC]\t$@ <= $^"; $(CROSS_COMPILE)gcc
LD = $(Q)echo "[LD]\t$@ <= $^"; $(CROSS_COMPILE)ld
AR = $(Q)echo "[AR]\t$@ <= $^"; $(CROSS_COMPILE)ar
MKDIR = $(Q)mkdir -p
RM = $(Q)rm -rf
SED = $(Q)echo "[SED]\t$@ <= $^"; sed
FOR = $(Q)for

CFLAGS +=	-Wall -Wundef -Werror -Wstrict-prototypes -Wno-trigraphs \
			-Werror-implicit-function-declaration -Wno-format-security \
			-fno-strict-aliasing -g -O2 \
			-I$(INCLUDE_PATH) -I. -DCAVAN_ARCH=$(ARCH)
ASFLAGS +=	$(CFLAGS) -D__ASM__
LDFLAGS += -lm -lpthread

define file_path_convert
$(patsubst $(1)/%$(2),$(3)%$(4),$(wildcard $(1)/*$(2)))
endef

LIB_OBJ_FILES = $(call file_path_convert,$(LIB_PATH),.c,$(OUT_LIB)/,.o)
TARGET_LIBO = $(OUT_LIB)/$(CAVAN_NAME).o
TARGET_LIBSO = $(OUT_LIB)/lib$(CAVAN_NAME).so
TARGET_LIBA = $(OUT_LIB)/lib$(CAVAN_NAME).a

APP_FILENAMES = $(call file_path_convert,$(APP_PATH),.c)
APP_OBJ_FILES = $(patsubst %,$(OUT_APP)/%.o,$(APP_FILENAMES))
TARGET_BINS = $(addprefix $(OUT_BIN)/$(CAVAN_NAME)-,$(APP_FILENAMES))
ifeq ($(BUILD_TYPE),debug)
CFLAGS += -DCAVAN_DEBUG
BIN_DEPENDS = $(TARGET_LIBSO)  
APP_DEPENDS =
APP_LDFLAGS += -L$(ROOT_PATH)/$(OUT_LIB) -Wl,-rpath,$(ROOT_PATH)/$(OUT_LIB) -l$(CAVAN_NAME)
else
ifeq ($(BUILD_TYPE),static)
CFLAGS += -DCAVAN_STATIC
APP_DEPENDS = $(TARGET_LIBA)
BIN_DEPENDS =
APP_LDFLAGS += -static -L$(OUT_LIB) -l$(CAVAN_NAME)
else
ifeq ($(BUILD_TYPE),release)
CFLAGS += -DCAVAN_RELEASE
APP_DEPENDS = $(TARGET_LIBO)
BIN_DEPENDS =
APP_LDFLAGS += $(TARGET_LIBO)
else
$(error unknown build type: $(BUILD_TYPE))
endif
endif
endif

CAVAN_SRC_FILES = $(patsubst %,$(OUT_CAVAN)/%.c,$(APP_FILENAMES))
APP_CORE_OBJ_FILES = $(call file_path_convert,$(APP_CORE_PATH),.c,$(OUT_CAVAN)/,.o)
CAVAN_OBJ_FILES = $(CAVAN_SRC_FILES:%.c=%.o) $(APP_CORE_OBJ_FILES)
CAVAN_MAP_HEADER = $(OUT_CAVAN)/cavan_map.h
CAVAN_MAP_SOURCE = $(OUT_CAVAN)/cavan_map.c
TARGET_CAVAN = $(OUT_BIN)/$(CAVAN_NAME)-main
CAVAN_CFLAGS = -I$(ROOT_PATH)/$(OUT_CAVAN)

all: $(BUILD_ENTRY)

app: build_env $(BIN_DEPENDS) $(TARGET_BINS)

$(OUT_BIN)/$(CAVAN_NAME)-%: $(OUT_APP)/%.o $(APP_DEPENDS)
	$(CC) -o $@ $< $(LDFLAGS) $(APP_LDFLAGS) $(LDFLAGS)

$(OUT_APP)/%.o: $(APP_PATH)/%.c
	$(CC) $(CFLAGS) -o $@ -c $^

lib: build_env $(TARGET_LIBA) $(TARGET_LIBSO)

liba: build_env $(TARGET_LIBA)

libso: build_env $(TARGET_LIBSO)

$(TARGET_LIBA): $(TARGET_LIBO)
	$(AR) cur $@ $^

$(TARGET_LIBSO): $(TARGET_LIBO)
	$(CC) -shared -o $@ $^

$(TARGET_LIBO): $(LIB_OBJ_FILES)
	$(LD) -o $@ -r $^

$(OUT_LIB)/%.o: $(LIB_PATH)/%.c
	$(CC) $(CFLAGS) -fPIC -o $@ -c $^

cavan: build_env $(BIN_DEPENDS) $(TARGET_CAVAN)

$(TARGET_CAVAN): $(CAVAN_OBJ_FILES) $(APP_DEPENDS)
	$(CC) -o $@ $^ $(APP_LDFLAGS) $(LDFLAGS)

$(OUT_CAVAN)/%.o: $(OUT_CAVAN)/%.c
	$(CC) $(CFLAGS) -o $@ -c $^

$(OUT_CAVAN)/%.c: $(APP_PATH)/%.c
	$(eval main-name = $(patsubst $(APP_PATH)/%.c,do_cavan_%,$^))
	$(SED)	-e "s/^\s*void\s\+main\s*(\(.*\))\s*$$/void $(main-name)(\1)/g" \
			-e "s/^\s*int\s\+main\s*(\(.*\))\s*$$/int $(main-name)(\1)/g" \
			$^ > $@

$(OUT_CAVAN)/%.o: $(APP_CORE_PATH)/%.c $(CAVAN_MAP_HEADER) $(CAVAN_MAP_SOURCE)
	$(CC) $(CFLAGS) $(CAVAN_CFLAGS) -o $@ -c $<

$(CAVAN_MAP_HEADER): $(CAVAN_SRC_FILES)
	$(FOR) app in $(patsubst $(OUT_CAVAN)/%.c,do_cavan_%,$^); \
	do \
		echo "int $${app}(int argc, char *argv[]);"; \
	done > $@

$(CAVAN_MAP_SOURCE): $(CAVAN_SRC_FILES)
	$(FOR) app in $(patsubst $(OUT_CAVAN)/%.c,%,$^); \
	do \
		echo "{\"$${app}\", do_cavan_$${app}},"; \
	done > $@

clean:
	$(RM) $(OUT_PATH) -rfv

build_env:
	$(MKDIR) $(OUT_LIB) $(OUT_APP) $(OUT_BIN) $(OUT_CAVAN)

.PHONY: $(build_env)
