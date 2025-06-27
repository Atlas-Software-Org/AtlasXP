.PHONY: all build clean

SRC = printf.c stdlib.c stdio.c string.c
OBJ = $(SRC:.c=.o)
AR  = ar
CFLAGS = -static -fpie -fPIC -ffreestanding -nostdlib -nolibc

all: build

build: libc.a

libc.a: $(OBJ)
	@$(AR) rcs $@ $^

%.o: %.c
	@gcc $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) libc.a
