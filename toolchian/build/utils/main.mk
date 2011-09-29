SRC_UTILS = $(SRC_PATH)/utils
OUT_UTILS = $(OUT_PATH)/utils

GMP_NAME = gmp-$(GMP_VERSION)
SRC_GMP = $(SRC_UTILS)/$(GMP_NAME)
OUT_GMP = $(OUT_UTILS)/$(GMP_NAME)
MARK_GMP = $(MARK_UTILS)/$(GMP_NAME)
GMP_OPTION = --disable-shared

MPFR_NAME = mpfr-$(MPFR_VERSION)
SRC_MPFR = $(SRC_UTILS)/$(MPFR_NAME)
OUT_MPFR = $(OUT_UTILS)/$(MPFR_NAME)
MARK_MPFR = $(MARK_UTILS)/$(MPFR_NAME)
MPFR_OPTION = $(GMP_OPTION) --with-gmp=$(UTILS_PATH)/usr

MPC_NAME = mpc-$(MPC_VERSION)
SRC_MPC = $(SRC_UTILS)/$(MPC_NAME)
OUT_MPC = $(OUT_UTILS)/$(MPC_NAME)
MARK_MPC = $(MARK_UTILS)/$(MPC_NAME)
MPC_OPTION = $(MPFR_OPTION) --with-mpfr=$(UTILS_PATH)/usr --with-gnu-ld

GAWK_NAME = gawk-$(GAWK_VERSION)
SRC_GAWK = $(SRC_UTILS)/$(GAWK_NAME)
OUT_GAWK = $(OUT_UTILS)/$(GAWK_NAME)
MARK_GAWK = $(MARK_UTILS)/$(GAWK_NAME)

include $(MAKEFILE_DEFINES)

all: build_env $(MARK_GAWK) $(MARK_MPFR)

# $(MARK_MPC)

$(MARK_MPC): $(MARK_MPFR)
	$(call decompression_file,$(SRC_MPC))
	$(call remake_directory,$(OUT_MPC))
	$(Q)+make -C $(OUT_MPC) -f $(MAKEFILE_HOST_RULE) SRC=$(SRC_MPC)  OPT="$(MPC_OPTION)"
	$(call generate_mark)

$(MARK_MPFR): $(MARK_GMP)
	$(call decompression_file,$(SRC_MPFR))
	$(call remake_directory,$(OUT_MPFR))
	$(Q)+make -C $(OUT_MPFR) -f $(MAKEFILE_HOST_RULE) SRC=$(SRC_MPFR) OPT="$(MPFR_OPTION)"
	$(call generate_mark)

$(MARK_GMP):
	$(call decompression_file,$(SRC_GMP))
	$(call remake_directory,$(OUT_GMP))
	$(Q)+make -C $(OUT_GMP) -f $(MAKEFILE_HOST_RULE) SRC=$(SRC_GMP) OPT="$(GMP_OPTION)"
	$(call generate_mark)

$(MARK_GAWK):
	$(call decompression_file,$(SRC_GAWK))
	$(call remake_directory,$(OUT_GAWK))
	$(Q)+make -C $(OUT_GAWK) -f $(MAKEFILE_HOST_RULE) SRC=$(SRC_GAWK)
	$(call generate_mark)

build_env:
	$(Q)mkdir $(SRC_UTILS) $(OUT_UTILS) -pv

.PHONY: build_env
