#include "debug.h"

void e9debugk(char* str) {
    while (*str) {
        outb(0xE9, *str);
        str++;
    }
}

void e9debugkn(char* str, int size) {
    int s = size;
    while (*str && s) {
        outb(0xE9, *str);
        str++;
        s--;
    }
}

int int_to_str(int num, char* buf) {
    int i = 0, is_negative = 0;
    
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    char temp[16];
    int temp_i = 0;

    do {
        temp[temp_i++] = '0' + (num % 10);
        num /= 10;
    } while (num > 0);

    if (is_negative) {
        temp[temp_i++] = '-';
    }

    while (temp_i > 0) {
        buf[i++] = temp[--temp_i];
    }

    return i;
}

int hex_to_str(uint64_t num, char* buf) {
    const char* hex_chars = "0123456789ABCDEF";
    char temp[16];
    int i = 0, temp_i = 0;

    do {
        temp[temp_i++] = hex_chars[num % 16];
        num /= 16;
    } while (num > 0);

    while (temp_i > 0) {
        buf[i++] = temp[--temp_i];
    }

    return i;
}

void e9debugkf(const char* fmt, ...) {
    char buffer[256];  // Temporary buffer for formatted output
    int buf_index = 0;
    
    va_list args;
    va_start(args, fmt);

    for (int i = 0; fmt[i] != '\0' && buf_index < sizeof(buffer) - 1; i++) {
        if (fmt[i] == '%' && fmt[i + 1] != '\0') {
            i++;  // Move to the format specifier

            if (fmt[i] == 'd') {  // Integer
                int num = va_arg(args, int);
                buf_index += int_to_str(num, &buffer[buf_index]);
            } 
            else if (fmt[i] == 'x') {  // Hexadecimal
                uint64_t num = va_arg(args, uint64_t);
                buf_index += hex_to_str(num, &buffer[buf_index]);
            }
            else if (fmt[i] == 'p') {  // Pointer (Hex)
                uintptr_t ptr = va_arg(args, uintptr_t);
                buffer[buf_index++] = '0';
                buffer[buf_index++] = 'x';
                buf_index += hex_to_str(ptr, &buffer[buf_index]);
            }
            else if (fmt[i] == 's') {  // String
                const char* str = va_arg(args, const char*);
                while (*str && buf_index < sizeof(buffer) - 1) {
                    buffer[buf_index++] = *str++;
                }
            } 
            else {  // Unsupported format, print as-is
                buffer[buf_index++] = '%';
                buffer[buf_index++] = fmt[i];
            }
        } else {
            buffer[buf_index++] = fmt[i];
        }
    }

    buffer[buf_index] = '\0';  // Null-terminate the string
    va_end(args);

    e9debugk(buffer);  // Send formatted string to your debug function
}