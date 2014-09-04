CAVAN_BUILD_ARCH = $(shell uname -m)
CAVAN_BUILD_PLAT = $(shell gcc -dumpmachine)
CAVAN_TARGET_ARCH = arm
CAVAN_TARGET_EABI = gnueabi
# CAVAN_TARGET_EABI = androideabi

ifeq ($(CAVAN_TARGET_ARCH),arm)
  CPU_BINUTILS_OPTION = --with-float=soft
  CPU_GCC_OPTION = --with-arch=armv5te --with-float=soft --with-fpu=vfp --with-abi=aapcs --enable-target-optspace
endif

Q = @

GCC_VERSION = 4.9.1

DOWNLOAD_TYPES = tar.xz tar.bz2 tar.gz
PACKAGE_TYPES = $(DOWNLOAD_TYPES) tar zip rar
HOST_APPS = xz-utils m4 gawk gettext gcc g++
FORCE_DECOMPRESSION = no

DOWNLOAD_WAY = wget
SERVER_IP = 192.168.0.144
SERVER_PORT = 8888
SERVER_SOURCE = /work/source

APT_GET = sudo apt-get install -y

export CAVAN_TARGET_ARCH CAVAN_BUILD_ARCH CAVAN_BUILD_PLAT
export PACKAGE_VERSION_STRING
export CPU_BINUTILS_OPTION CPU_GCC_OPTION Q
export DOWNLOAD_TYPES FORCE_DECOMPRESSION PACKAGE_TYPES
export DOWNLOAD_WAY SERVER_IP SERVER_PORT SERVER_SOURCE
export APT_GET

include build/core/main.mk
