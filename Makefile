ARCH = host
CROSS_COMPILE =
BUILD_TYPE = debug
BUILD_ENTRY = app
Q = @
BUILD_MAIN = main2

include build/core/$(BUILD_MAIN).mk
