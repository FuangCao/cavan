BUILD_TYPE = dynamically
# BUILD_TYPE = release
# BUILD_TYPE = static
ARCH = x86
CROSS_COMPILE =
ELF_PREFIX = cavan
Q = @
LDFLAGS = -lpthread

export BUILD_TYPE CROSS_COMPILE ARCH ELF_PREFIX Q

include build/core/main.mk
