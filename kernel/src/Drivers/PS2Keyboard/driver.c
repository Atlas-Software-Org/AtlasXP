/*
    Author: Adam Bassem
    Revision 0
    Patch 1
    Minor 0
    Major 0
    Atlas 0.0.7
*/

#include <KiSimple.h>
#include <PMM/pmm.h>
#include <IDT/idt.h>

#define KBD_BUF_SIZE 65535

char kbd_drvr_buf[KBD_BUF_SIZE];
volatile uint16_t kbd_buf_head = 0;
volatile uint16_t kbd_buf_tail = 0;
bool buffering_read = false;

volatile int kbd_scrolled_up = 0;
volatile int kbd_scrolled_down = 0;

int KiReadHidC() {
    buffering_read = true;
    while (kbd_buf_head == kbd_buf_tail);
    char c = kbd_drvr_buf[kbd_buf_tail];
    kbd_buf_tail = (kbd_buf_tail + 1) % KBD_BUF_SIZE;
    buffering_read = false;
    return c;
}

int KiReadHidSN(char *out, int maxlen) {
    int read = 0;
    while (read < maxlen - 1) {
        char c = KiReadHidC();
        if (c == '\n' || c == '\0')
            break;
        out[read++] = c;
    }
    out[read] = '\0';
    return read;
}

int KiReadHidS(char *out) {
    return KiReadHidSN(out, 8192);
}

static void OverflowKbdBfr() {
    kbd_buf_head = 0;
    kbd_buf_tail = 0;
    for (int i = 0; i < KBD_BUF_SIZE; i++) {
        kbd_drvr_buf[i] = 0;
    }
}

void InitKeyboardDriver() {
    kbd_buf_head = 0;
    kbd_buf_tail = 0;
}

static void KbdPushback(char c) {
    uint16_t next = (kbd_buf_head + 1) % KBD_BUF_SIZE;
    if (next != kbd_buf_tail) {
        kbd_drvr_buf[kbd_buf_head] = c;
        kbd_buf_head = next;
        printk("%c", c);
    } else {
        OverflowKbdBfr();
        KbdPushback(c);
    }
}

static void KbdNAFPushback(char c) {
    uint16_t next = (kbd_buf_head + 1) % KBD_BUF_SIZE;
    if (next != kbd_buf_tail) {
        kbd_drvr_buf[kbd_buf_head] = c;
        kbd_buf_head = next;
    } else {
        OverflowKbdBfr();
        KbdNAFPushback(c);
    }
}

uint8_t __global_keyboard_autoflush = 0; // 0 = autoflush enabled, 1 = disabled

static uint8_t shift = 0;
static uint8_t caps = 0;

void KbdFlushCheck(char chr) {
    if (__global_keyboard_autoflush == 0) {
        if (chr == '\b') {
            KbdPushback(0);
        }
        KbdPushback(chr);
    } else {
        if (chr == '\b') {
            KbdNAFPushback(0);
        }
        KbdNAFPushback(chr);
    }
}

char USLayoutNrml[128] = {
    0, '`', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

char USLayoutCaps[128] = {
    0, '`', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', 0, 0,
    '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

char USLayoutShft[128] = {
    0, '~', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '`', 0,
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void KeyboardDriverMain(uint8_t scancode) {
    static bool extended = false;
    static char ch;

    if (scancode == 0xE0) {
        extended = true;
        return;
    }

    if (extended) {
        if (scancode & 0x80) {
            extended = false;
            return;
        }

        int old_autoflush = __global_keyboard_autoflush;
        __global_keyboard_autoflush = 1;

        if (buffering_read == true) {
            __global_keyboard_autoflush = old_autoflush;
            extended = false;
            return;
        }
        switch (scancode) {
            case 0x48: // Up arrow
                KbdFlushCheck('\x1B');
                KbdFlushCheck('[');
                KbdFlushCheck('A');
                printk("\x1B[A");
                break;
            case 0x50: // Down arrow
                KbdFlushCheck('\x1B');
                KbdFlushCheck('[');
                KbdFlushCheck('B');
                printk("\x1B[B");
                break;
            case 0x4B: // Left arrow
                KbdFlushCheck('\x1B');
                KbdFlushCheck('[');
                KbdFlushCheck('D');
                printk("\x1B[D");
                break;
            case 0x4D: // Right arrow
                KbdFlushCheck('\x1B');
                KbdFlushCheck('[');
                KbdFlushCheck('C');
                printk("\x1B[C");
                break;
        }

        __global_keyboard_autoflush = old_autoflush;
        extended = false;
        return;
    }

    if (scancode & 0x80) {
        uint8_t key = scancode & 0x7F;
        if (key == 0x2A || key == 0x36) {
            shift = false;
        }
        return;
    } else {
        if (scancode == 0x2A || scancode == 0x36) {
            shift = true;
            return;
        }
        if (scancode == 0x3A) {
            caps = !caps;
            return;
        }
    }

    if (shift) {
        ch = USLayoutShft[scancode];
    } else if (caps && USLayoutNrml[scancode] >= 'a' && USLayoutNrml[scancode] <= 'z') {
        ch = USLayoutCaps[scancode];
    } else {
        ch = USLayoutNrml[scancode];
    }

    if (ch != 0) {
        KbdFlushCheck(ch);
    }
}
