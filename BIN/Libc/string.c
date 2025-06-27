#include "string.h"

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
