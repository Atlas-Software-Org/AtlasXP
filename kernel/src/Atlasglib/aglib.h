//
// Created by Adam on 2/3/2025.
//

#pragma once

#ifndef AGLIB_H
#define AGLIB_H

#include <stdint.h>
#include <stddef.h>

extern void* FbPtr_;
extern uint32_t pitch;

struct GraphicsHandle {
    uint32_t Width;
    uint32_t Height;
    uint32_t BPP;
    uint32_t Pitch;
    void* FbPtr;
    uint32_t PitchDBpp;
};

class GraphicsLibrary {
private:
    GraphicsHandle _GraphicsHandle;
    static int GraphicsHandleLineWidth; // Removed initialization

public:
    GraphicsLibrary(uint32_t w, uint32_t h, uint32_t bpp, uint32_t p, volatile uint32_t* ptr, bool UseAlpha);

    uint32_t alpha_blend(uint32_t source, uint32_t background);
    uint32_t GetPixel(uint32_t x, uint32_t y);
    void PutPixel(uint32_t x, uint32_t y, uint32_t color);

    void aglib_draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color);
    void aglib_draw_dotted_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t dotlen, uint32_t color);
    void aglib_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
    void aglib_draw_filled_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
    int aglib_abs(int x);
    void aglib_draw_circle(uint32_t x, uint32_t y, uint32_t radius, uint32_t color);
    void aglib_draw_filled_circle(uint32_t x, uint32_t y, uint32_t radius, uint32_t color);
    void aglib_draw_text(uint32_t x, uint32_t y, const char* text, uint8_t* font, uint32_t color);
    void aglib_data_set_line_width(int width);
    void aglib_data_line_width_reset();
};

void GlobPutPixel(uint32_t x, uint32_t y, uint32_t color);

#endif // AGLIB_H
