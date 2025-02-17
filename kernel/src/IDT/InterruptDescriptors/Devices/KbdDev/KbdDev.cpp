//
// Created by Adam on 2/7/2025.
//

#include "KbdDev.h"
#include <FontRenderer.h>

        // Initialization of key layouts for US Keyboard
        char KbdUSLayout[256] = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            ' ', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t', 'q',
            'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's', 'd',
            'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b',
            'n', 'm', ',', '.', '/', 0, '*', 0, ' '
        };
    
        char KbdUSLayout2[256] = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            ' ', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t', 'Q',
            'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0, 'A', 'S', 'D',
            'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|', 'Z', 'X', 'C', 'V', 'B',
            'N', 'M', '<', '>', '?', 0, '*', 0, ' '
        };
    
        char KbdUSLayout3[256] = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            ' ', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\b', '\t', 'A',
            'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', 0, '\\', 'Z', 'X', 'C',
            'V', 'B', 'N', 'M', ',', '.', '/', 0, '*', 0, ' '
        };
        static char inputBuffer[32768];
        static int bufferIndex;

        static bool RenderChar = false;
        static uint32_t x = 0;
        static uint32_t y = 0;

void SetupKbd(bool RenderChar__, uint32_t x__, uint32_t y__) {
    RenderChar = RenderChar__;
    x = x__;
    y = y__;
}

// Keyboard states and buffer initialization
bool CAPS = false;
bool SHIFT = false;
bool Reading = false;

// Interrupt handler for keyboard input
void KbdDevInterrupt(/*InterruptFrame* Frame*/) {
    uint8_t ScanCode = inb(0x60);  // Read scan code from port 0x60
    char CorrespondingChar;
    char* Layout = KbdUSLayout;  // Default layout

    switch (ScanCode) {
        case 0x3A:  // CAPS LOCK
            CAPS = !CAPS;
            break;
        case 0x2A:  // SHIFT press
            SHIFT = true;
            break;
        case 0xAA:  // SHIFT release
            SHIFT = false;
            break;
        default:
            break;
    }

    // Select the correct layout based on SHIFT and CAPS
    if (SHIFT) {
        Layout = KbdUSLayout2;
    } else if (CAPS) {
        Layout = KbdUSLayout3;
    }

    // Translate scan code to character
    if (ScanCode < 256) {
        CorrespondingChar = Layout[ScanCode];
        if (Reading) {
            inputBuffer[bufferIndex++] = CorrespondingChar;
            if (bufferIndex >= 32768) {
                bufferIndex = 0;  // Reset the buffer if full
                for (int i = 0; i < 32768; i++) {
                    inputBuffer[i] = 0;  // Clear the buffer
                }
            }
            if (RenderChar) {
                font_char(CorrespondingChar, x, y, 0xFFFFFFFF);
                outb(0xE9, CorrespondingChar);
            }
        }
    }
}

// Function to get a character from the input buffer
size_t KbdDevGetChr(char c) {
    Reading = true;
    while (bufferIndex == 0);  // Wait until there's data in the buffer
    c = inputBuffer[bufferIndex++];
    Reading = false;
    return 1;
}

// Function to get a string from the input buffer (limited by 'count')
size_t KbdDevGetStrN(char* s, size_t* count) {
    size_t counter = *count;
    size_t bufferi = 0;
    while (counter--) {
        KbdDevGetChr(s[bufferi]);
        if (s[bufferi] == '\n') {
            s[bufferi] = '\0';  // Null-terminate the string
            break;
        }
        bufferi++;
    }
    return bufferi;
}

// Function to get a string from the input buffer (up to 32768 characters)
size_t KbdDevGetStr(char* s) {
    size_t counter = 32768;
    size_t bufferi = 0;
    while (counter--) {
        KbdDevGetChr(s[bufferi]);
        if (s[bufferi] == '\n') {
            s[bufferi] = '\0';  // Null-terminate the string
            break;
        }
        bufferi++;
    }
    return bufferi;
}

__attribute__((interrupt)) extern "C" void KbdDevInterruptHandler(InterruptFrame* FRAME) {
    KbdDevInterrupt();
    outb(0x20, 0x20);
}
