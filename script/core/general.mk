define list_target
$(patsubst %.$1,%,$(wildcard *.$1))
endef

AS = $(CROSS_COMPILE)as
CC = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar
DESTDIR = /usr
PREFIX = cavan-

TARGET_FILE = $(call list_target,c) $(call list_target,cpp)

CFLAGS += -Wall -Werror -I. -I /cavan/include
CPPFLAGS += $(CFLAGS)

all: $(TARGET_FILE)

%: %.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

%: %.cpp
	$(CCP) $(CPPFLAGS) $^ $(LDFLAGS) -o $@

clean:
	@rm a.out *.o $(TARGET_FILE) -rfv

$(DESTDIR)/bin/$(PREFIX)%: %
	@install -cv $^ $@

install: $(addprefix $(DESTDIR)/bin/$(PREFIX),$(TARGET_FILE))

uninstall:
	@rm -rfv $(addprefix $(DESTDIR)/bin/$(PREFIX),$(TARGET_FILE))
