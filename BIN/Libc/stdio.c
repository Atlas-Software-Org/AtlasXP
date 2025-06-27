#include "stdio.h"
#include "../LibAxpApi/AxpApi.h"

FILE* stdin = (FILE*)0;
FILE* stdout = (FILE*)1;
FILE* stderr = (FILE*)2;

int fputc(int c, FILE* f) {
    char ch = (char)c;
    return write((int)(intptr_t)f, &ch, 1);
}

int fputs(const char* s, FILE* f) {
    int len = 0;
    while (s[len]) len++;
    return write((int)(intptr_t)f, (void*)s, len);
}

int fgetc(FILE* f) {
	(void)f;
    char c;
    char buf[2];
    int r = read(0, buf, 1);
    c = buf[0];
    return r == 1 ? (int)c : EOF;
}

char* fgets(char* buf, int size, FILE* f) {
	(void)f;
	read(0, buf, size);

    return buf;
}

void puts(const char* s) {
    fputs(s, stdout);
    fputc('\n', stdout);
}

void putchar(char c) {
    fputc(c, stdout);
}

void termgetres(unsigned long long* width, unsigned long long* height) {
	*width = api_get_term_width();
	*height = api_get_term_height();
}
