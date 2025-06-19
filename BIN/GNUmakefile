all: build genf2c

build:
	@echo "  CC\t\tnutshell.c\tLibAxpApi.c\tAxpCrt.c\t-o nutshell"
	@gcc -static -fpie -fPIC -ffreestanding -nostdlib -nolibc NUTSHELL/nutshell.c LibAxpApi/LibAxpApi.c AxpCrt.c -e _start -o OUT/NUTSHELL/nutshell

genf2c:
	@echo "  GENF2C\t../kernel/src/BIN/nutshell.h"
	@genf2c -f OUT/NUTSHELL/nutshell -o ../kernel/src/BIN/nutshell.h

clean:
	rm -rf ./OUT/NUTSHELL/*