#include "../LibAxpApi/AxpApi.h"

int main(int argc, char** argv, char** envp) {
    (void)argc;(void)argv;(void)envp;

    write(1, "Hello, World!\n", 14);

    char buf[4096] = {0};
    while (1) {
        write(1, "nutshell> ", 10);
        int readc = read(0, buf, 4096);
        write(1, "Nice command! ->\n\r", 18);
        write(1, buf, readc);
        buf[0] = 0;
    }
}