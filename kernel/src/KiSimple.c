#include <KiSimple.h>
#include <stdint.h>

void outb(uint16_t port, uint8_t byte) {
    asm volatile ("outb %0, %1" : : "a"(byte), "dN"(port));
}
uint8_t inb(uint16_t port) {
    uint8_t result;
    asm volatile ("inb %1, %0" : "=a"(result) : "dN"(port));
    return result;
}
void outw(uint16_t port, uint16_t word) {
    asm volatile ("outw %0, %1" : : "a"(word), "dN"(port));
}
uint16_t inw(uint16_t port) {
    uint16_t result;
    asm volatile ("inw %1, %0" : "=a"(result) : "dN"(port));
    return result;
}
void outl(uint16_t port, uint32_t dword) {
    asm volatile ("outl %0, %1" : : "a"(dword), "dN"(port));
}
uint32_t inl(uint16_t port) {
    uint32_t result;
    asm volatile ("inl %1, %0" : "=a"(result) : "dN"(port));
    return result;
}

void IOWait() {
    outb(0x80, 0x00);
}

inline void insw(uint16_t port, void* addr, int count) {
    asm volatile ("rep insw"
                  : "+D"(addr), "+c"(count) // Destination address and counter
                  : "d"(port)               // Source port
                  : "memory");              // Clobbers memory
}

inline void outsw(uint16_t port, const void* addr, int count) {
    asm volatile ("rep outsw"
                  : "+S"(addr), "+c"(count) // Source address and counter
                  : "d"(port));             // Destination port
}

void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
    uint8_t *restrict pdest = (uint8_t *restrict)dest;
    const uint8_t *restrict psrc = (const uint8_t *restrict)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

char* strcpy(char* dest, const char* src) {
    char* ptr = dest;
    while ((*ptr++ = *src++));
    return dest;
}

char* strncpy(char* dest, const char* src, unsigned int n) {
    char* ptr = dest;
    unsigned int i = 0;
    while (i < n && src[i]) {
        ptr[i] = src[i];
        i++;
    }
    while (i < n) {
        ptr[i++] = '\0';
    }
    return dest;
}

unsigned int strlen(const char* str) {
    unsigned int len = 0;
    while (str[len]) len++;
    return len;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

int strncmp(const char* s1, const char* s2, unsigned int n) {
    unsigned int i = 0;
    while (i < n && s1[i] && (s1[i] == s2[i])) {
        i++;
    }
    if (i == n) return 0;
    return (unsigned char)s1[i] - (unsigned char)s2[i];
}

char* strcat(char* dest, const char* src) {
    char* ptr = dest;
    while (*ptr) ptr++;
    while ((*ptr++ = *src++));
    return dest;
}

char* strncat(char* dest, const char* src, unsigned int n) {
    char* ptr = dest;
    unsigned int i = 0;
    while (*ptr) ptr++;
    while (i < n && src[i]) {
        *ptr++ = src[i++];
    }
    *ptr = '\0';
    return dest;
}

char* strchr(const char* str, int c) {
    while (*str) {
        if (*str == (char)c) return (char*)str;
        str++;
    }
    return NULL;
}

char* strstr(const char* haystack, const char* needle) {
    if (!*needle) return (char*)haystack;
    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && (*h == *n)) {
            h++;
            n++;
        }
        if (!*n) return (char*)haystack;
        haystack++;
    }
    return NULL;
}

char* strtok(char* str, const char* delim) {
    static char* next;
    if (str) next = str;
    if (!next) return NULL;

    // Skip leading delimiters
    char* start = next;
    while (*start && strchr(delim, *start)) start++;
    if (!*start) {
        next = NULL;
        return NULL;
    }

    // Find end of token
    char* end = start;
    while (*end && !strchr(delim, *end)) end++;

    if (*end) {
        *end = '\0';
        next = end + 1;
    } else {
        next = NULL;
    }

    return start;
}

static void halt_catch_fire_x86() {
    while (1) {asm volatile ("hlt");}
}

void KiPanic(const char* __restrict string, int _halt) {
	if (_halt == 1) {
    	printk("\x1b[1;91m{ PANIC }\t%s", string);
    } else {
    	printk("{ PANIC }\t%s", string);
    }
    
    printk("\n");
    if (_halt) halt_catch_fire_x86();
    else return;
}

void DisplaySplash(int w, int h, char* text) {
    int lf_count = 0;
    int i = 0;
    char *line_start = text;
    char *line_end = NULL;
    int line_len = 0;
    int line_num = 0;

    while (text[i]) {
        if (text[i] == '\n') lf_count++;
        i++;
    }

    int total_lines = lf_count + 1;
    int center_y = h / 2 - (total_lines / 2);

    char info[256] = {0};
    int info_len = 0;
    bool info_found = false;

    line_start = text;
    line_num = 0;
    while (line_start && *line_start) {
        line_end = strchr(line_start, '\n');
        if (!line_end) line_end = line_start + strlen(line_start);

        line_len = (int)(line_end - line_start);

        // Check if line ends with $INF< sequence after newline
        if (line_len >= 4 && line_start[0] == '$' &&
            line_start[1] == 'I' && line_start[2] == 'N' && line_start[3] == 'F' &&
            line_start[4] == '<') {
            // This line itself is info start, treat as info (unlikely standalone)
            info_found = true;
            info_len = line_len - 5;
            if (info_len > 0 && info_len < (int)sizeof(info)) {
                memcpy(info, line_start + 5, info_len);
                info[info_len] = '\0';
            }
            break;
        }

        // If newline found, check if next line starts with $INF<
        if (*line_end == '\n') {
            char *next_line = line_end + 1;
            if (next_line[0] == '$' && next_line[1] == 'I' && next_line[2] == 'N' &&
                next_line[3] == 'F' && next_line[4] == '<') {
                info_found = true;
                char *info_start = next_line + 5;
                char *info_end = strstr(info_start, ">INF$");
                if (info_end && info_end > info_start) {
                    info_len = (int)(info_end - info_start);
                    if (info_len > 0 && info_len < (int)sizeof(info)) {
                        memcpy(info, info_start, info_len);
                        info[info_len] = '\0';
                    }
                }
                // Print current line only (not the $INF line)
                int center_x = w / 2 - (line_len / 2);
                if (center_x < 0) center_x = 0;
                printk("\x1b[%d;%dH", center_y + line_num + 1, center_x + 1);
                for (i = 0; i < line_len; i++) printk("%c", line_start[i]);
                line_num++;
                break;
            }
        }

        int center_x = w / 2 - (line_len / 2);
        if (center_x < 0) center_x = 0;

        printk("\x1b[%d;%dH", center_y + line_num + 1, center_x + 1);

        for (i = 0; i < line_len; i++) {
            printk("%c", line_start[i]);
        }

        line_num++;

        if (*line_end == '\n') line_start = line_end + 1;
        else break;
    }

    if (info_found) {
        // One line padding before info
        printk("\x1b[%d;1H", center_y + line_num + 2);

        int info_len_local = (int)strlen(info);
        int center_x = w / 2 - (info_len_local / 2);
        if (center_x < 0) center_x = 0;
        printk("\x1b[%d;%dH", center_y + line_num + 2, center_x + 1);
        printk("\x1b[90m%s\x1b[0m", info);
    }

    printk("\e[2J\e[H");
}
