all: clean libc build genf2c

SHARED_CC_FLAGS := -I./Libc/ ./Libc/libc.a

libc:
	$(MAKE) -C ./Libc/

build:
	@mkdir -p OUT OUT/NUTSHELL

	@echo "  CC\t\tnutshell.c\tLibAxpApi.c\tAxpCrt.c\t-o nutshell"

	@gcc -static -fpie -fPIC -ffreestanding -nostdlib -nolibc NUTSHELL/nutshell.c LibAxpApi/LibAxpApi.c AxpCrt.c -e _start -o OUT/NUTSHELL/nutshell $(SHARED_CC_FLAGS)

genf2c:
	@echo "  GENF2C\t../kernel/src/BIN/nutshell.h"
	@mv OUT/NUTSHELL/nutshell .
	@genf2c -f nutshell -o ../kernel/src/BIN/nutshell.h
	@mv nutshell OUT/NUTSHELL/nutshell

clean:
	rm -rf OUT

run: all
	$(MAKE) -C ../ run
