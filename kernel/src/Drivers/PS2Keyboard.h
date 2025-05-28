/*
    Author: Adam Bassem
    Revision 0
    Patch 0
    Minor 0
    Major 0
    Atlas 0.0.7
*/

#ifndef PS2KEYBOARD_H
#define PS2KEYBOARD_H 1

void InitKeyboardDriver();
static void KbdPushback(char c);
static void KbdNAFPushback(char c);

void KeyboardDriverMain(uint8_t sc);

int KiReadHidC();
int KiReadHidSN(char *out, int maxlen);
/* Read 8192 bytes by default */
int KiReadHidS(char *out);

#endif /* PS2KEYBOARD_H */