define list_target
$(patsubst %.$1,%,$(wildcard *.$1))
endef

AS = $(CROSS_COMPILE)as
CC = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar

CTARGET = $(call list_target,c)
CPPTARGET = $(call list_target,cpp)

CFLAGS += -Wall -Werror
CPPFLAGS += $(CFLAGS)

all: $(CTARGET) $(CPPTARGET)

%: %.c

%: %.cpp

clean:
	@rm a.out *.o $(CTARGET) $(CPPTARGET) -rfv
