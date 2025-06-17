#ifndef PS2KEYBOARD_H
#define PS2KEYBOARD_H 1

void InitKeyboardDriver();

void KeyboardDriverMain(uint8_t sc);

int KiReadHidC();
int KiReadHidSN(char *out, int maxlen);
/* Read 8192 bytes by default */
int KiReadHidS(char *out);

#endif /* PS2KEYBOARD_H */