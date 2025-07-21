#include "stdio.h"
#include "../LibAsnuApi/AsnuApi.h"
#include <stdint.h>

int printf(const char* fmt, ...) {
    char buf[1024];
    char* out = buf;
    const char* p = fmt;
    int written = 0;

    __builtin_va_list args;
    __builtin_va_start(args, fmt);

    while (*p) {
        if (*p == '%') {
            p++;
            if (*p == 's') {
                char* str = __builtin_va_arg(args, char*);
                while (*str) *out++ = *str++, written++;
            } else if (*p == 'd' || *p == 'i') {
                int64_t v = __builtin_va_arg(args, int64_t);
                char tmp[32];
                int i = 0, neg = v < 0;
                if (neg) v = -v;
                do {
                    tmp[i++] = '0' + (v % 10);
                    v /= 10;
                } while (v);
                if (neg) tmp[i++] = '-';
                while (i--) *out++ = tmp[i], written++;
            } else if (*p == 'u') {
                uint64_t v = __builtin_va_arg(args, uint64_t);
                char tmp[32];
                int i = 0;
                do {
                    tmp[i++] = '0' + (v % 10);
                    v /= 10;
                } while (v);
                while (i--) *out++ = tmp[i], written++;
            } else if (*p == 'x' || *p == 'X') {
                uint64_t v = __builtin_va_arg(args, uint64_t);
                const char* hex = (*p == 'x') ? "0123456789abcdef" : "0123456789ABCDEF";
                for (int i = 60; i >= 0; i -= 4)
                    *out++ = hex[(v >> i) & 0xF], written++;
            } else if (*p == 'c') {
                char c = (char)__builtin_va_arg(args, int);
                *out++ = c, written++;
            } else if (*p == 'p') {
                uintptr_t v = (uintptr_t)__builtin_va_arg(args, void*);
                const char* hex = "0123456789abcdef";
                *out++ = '0', *out++ = 'x', written += 2;
                for (int i = (sizeof(uintptr_t) * 8 - 4); i >= 0; i -= 4)
                    *out++ = hex[(v >> i) & 0xF], written++;
            } else if (*p == '%') {
                *out++ = '%', written++;
            } else {
                *out++ = '%', *out++ = *p, written += 2;
            }
        } else {
            *out++ = *p, written++;
        }
        p++;
    }

    *out = 0;
    __builtin_va_end(args);
    write(1, buf, written);
    return written;
}
