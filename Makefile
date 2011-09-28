BUILD_TYPE = dynamically
# BUILD_TYPE = release
# BUILD_TYPE = static
ARCH = x86
CROSS_COMPILE =
ELF_PREFIX = cavan
Q = @
LDFLAGS = -lpthread

include build/core/main.mk
