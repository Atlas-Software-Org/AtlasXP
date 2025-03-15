#pragma once

#include <stdint.h>
#include <limine.h>

#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

typedef struct {
    unsigned char magic[2];
    unsigned char mode;
    unsigned char charsize;
} PSF1_HEADER;

typedef struct {
    PSF1_HEADER* psf1_Header;
    uint8_t* glyphBuffer;
} PSF1_FONT;

extern PSF1_FONT* main_psf1_font;
extern uint32_t ClearColor;

void InitGfx(struct limine_framebuffer* fb);
struct limine_framebuffer *GetFb();
void PutPx(uint64_t x, uint64_t y, uint32_t clr);
uint32_t GetPx(uint64_t x, uint64_t y);
void DrawRect(uint64_t x, uint64_t y, uint64_t width, uint64_t len, uint32_t clr);

void FontPutChar(char c, uint64_t x, uint64_t y, uint32_t clr);
void FontPutStr(const char* s, uint64_t x, uint64_t y, uint32_t clr);