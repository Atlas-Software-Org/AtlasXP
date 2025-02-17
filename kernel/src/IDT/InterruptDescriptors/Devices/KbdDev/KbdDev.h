//
// Created by Adam on 2/7/2025.
//

#ifndef KBDDEV_H
#define KBDDEV_H

#include <stdint.h>
#include <stddef.h>
#include <IDTTools.h>
#include <io.h>

void KbdDevInterrupt();
size_t KbdDevGetChr(char c);
size_t KbdDevGetStrN(char* s, size_t *count);
size_t KbdDevGetStr(char* s);

__attribute__((interrupt)) extern "C" void KbdDevInterruptHandler(InterruptFrame* FRAME);

#endif //KBDDEV_H
