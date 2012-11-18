LOCAL_PATH = $(PWD)

CFLAGS = -Iinclude -Wall -Werror
LDFLAGS = -lpthread

LIB_SRC_FILES = $(wildcard libhuamobile/*.c)
LIB_OBJ_FILES = $(LIB_SRC_FILES:%.c=%.o)

$(info LIB_SRC_FILES = $(LIB_SRC_FILES))

hua_tstest: /tmp/hua_tstest

/tmp/hua_tstest: hua_tstest/hua_tstest.c $(LIB_OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm $(LIB_OBJ_FILES) -rfv
