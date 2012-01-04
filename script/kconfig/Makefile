CC = gcc
LDFLAGS = -lncurses
CFLAGS = -Werror -Wall

	#./$<; echo $$?
all: kconfig

kconfig: $(patsubst %.c,%.o,$(wildcard *.c))
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm *.o kconfig -rfv

.PHOHY: all
