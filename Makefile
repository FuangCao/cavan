BUILD_TYPE = dynamically
# BUILD_TYPE = release
# BUILD_TYPE = static
# ARCH =
# CROSS_COMPILE =
ELF_PREFIX = cavan
Q = @
LDFLAGS = -lpthread -lm

include build/core/main.mk
