#pragma once

#include <stdint.h>
#include <stdarg.h>
#include "io.h"

void e9debugk(char* str);
void e9debugkn(char* str, int size);
int int_to_str(int num, char* buf);
int hex_to_str(uint64_t num, char* buf);
void e9debugkf(const char* fmt, ...);