#include <gfx.h>
#include <mem/mem.h>

struct limine_framebuffer* main_fb;
PSF1_FONT* main_psf1_font;
uint32_t ClearColor;

void InitGfx(struct limine_framebuffer* fb) {
    main_fb = fb;
}

struct limine_framebuffer *GetFb() {
    return main_fb;
}

void PutPx(uint64_t x, uint64_t y, uint32_t clr) {
    volatile uint32_t *fb_ptr = main_fb->address;
    fb_ptr[y * (main_fb->pitch / (main_fb->bpp/8)) + x] = clr;
}

uint32_t GetPx(uint64_t x, uint64_t y) {
    volatile uint32_t *fb_ptr = main_fb->address;
    return (fb_ptr[y * (main_fb->pitch / (main_fb->bpp/8)) + x]);
}

void DrawRect(uint64_t x, uint64_t y, uint64_t width, uint64_t len, uint32_t clr) {
    for (uint64_t x__ = x; x__ < x + width; x__++) {
        for (uint64_t y__ = y; y__ < y + len; y__++) {
            PutPx(x__, y__, clr);
        }
    }
}

void FontPutChar(char c, uint64_t x, uint64_t y, uint32_t clr) {
    uint8_t* fontPtr = (uint8_t*)main_psf1_font->glyphBuffer + (c * main_psf1_font->psf1_Header->charsize);

    for (uint64_t row = 0; row < main_psf1_font->psf1_Header->charsize; row++) { // Iterate over each row of the character
        uint8_t pixelData = fontPtr[row]; // Each row contains 8 pixels

        for (uint64_t col = 0; col < 8; col++) { // Each row has 8 bits (pixels)
            if ((pixelData >> (7 - col)) & 1) { // Check if the bit is set
                PutPx(x + col, y + row, clr); // Draw pixel if bit is set
            }
        }
    }
}

void FontPutStr(const char* s, uint64_t x, uint64_t y, uint32_t clr) {
    uint64_t xoff = x;
    uint64_t yoff = y;

    // Calculate screen dimensions in character widths and heights
    uint64_t screenWidthChars = main_fb->width / 8;  // 8 pixels per character width
    uint64_t screenHeightChars = main_fb->height / 16;  // 16 pixels per character height

    while (*s) {
        char c = *s;

        if (c == '\b') {  // Handle backspace
            // Move the cursor back by one character
            if (xoff > x) {
                xoff -= 8;
            }
            DrawRect(xoff, yoff, 8, 16, ClearColor);
        } else if (c == '\n') {  // Handle newline
            // Move to the next line, reset x and increase y
            xoff = x;
            yoff += 16; // Assuming 16px height per line
        } else if (c == '\r') {  // Handle carriage return
            // Move the cursor back to the start of the line
            xoff = x;
        } else {  // Regular character rendering
            if (xoff + 8 > main_fb->width) {  // Check if the text exceeds the screen width
                xoff = x;  // Reset to the start of the next line
                yoff += 16;  // Move to the next line
            }

            if (yoff + 16 > main_fb->height) {  // Check if the text exceeds the screen height
                // You can either stop here, or you can reset to the top of the screen (scrolling behavior)
                // Uncomment the following line for scrolling behavior:
                // yoff = y;  // Reset to top of the screen
            }

            FontPutChar(c, xoff, yoff, clr);  // Print the character
            xoff += 8;  // Move the cursor to the right by 8 pixels
        }

        s++;
    }
}
