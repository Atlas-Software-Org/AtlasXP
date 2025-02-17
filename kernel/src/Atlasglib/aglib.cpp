//
// Created by Adam on 2/3/2025.
//

#include "aglib.h"

static bool AllowAlpha;
int GraphicsLibrary::GraphicsHandleLineWidth = 1;
void* FbPtr_ = 0;
uint32_t pitch = 1920;

// Constructor for initializing the graphics library
GraphicsLibrary::GraphicsLibrary(uint32_t w, uint32_t h, uint32_t bpp, uint32_t p, volatile uint32_t* ptr, bool UseAlpha) {
    _GraphicsHandle.Width = w;
    _GraphicsHandle.Height = h;
    _GraphicsHandle.BPP = bpp;
    _GraphicsHandle.Pitch = p;
    _GraphicsHandle.FbPtr = (void*)ptr;
    AllowAlpha = UseAlpha;
    _GraphicsHandle.PitchDBpp = p / bpp;
    FbPtr_ = (void*)ptr;
    pitch = p;
}

// Alpha blending function
uint32_t GraphicsLibrary::alpha_blend(uint32_t source, uint32_t background) {
    uint8_t alpha = (source >> 24) & 0xFF;
    uint8_t red_source = (source >> 16) & 0xFF;
    uint8_t green_source = (source >> 8) & 0xFF;
    uint8_t blue_source = source & 0xFF;

    uint8_t red_background = (background >> 16) & 0xFF;
    uint8_t green_background = (background >> 8) & 0xFF;
    uint8_t blue_background = background & 0xFF;

    uint8_t red_result = (red_source * alpha + red_background * (255 - alpha)) / 255;
    uint8_t green_result = (green_source * alpha + green_background * (255 - alpha)) / 255;
    uint8_t blue_result = (blue_source * alpha + blue_background * (255 - alpha)) / 255;

    uint32_t result = (alpha << 24) | (red_result << 16) | (green_result << 8) | blue_result;
    return result;
}

// Get the pixel color from framebuffer
uint32_t GraphicsLibrary::GetPixel(uint32_t x, uint32_t y) {
    return *((volatile uint32_t*)_GraphicsHandle.FbPtr + (y * (_GraphicsHandle.Pitch / 4) + x));
}

// Put a pixel on the framebuffer
void GraphicsLibrary::PutPixel(uint32_t x, uint32_t y, uint32_t color) {
    uint32_t FinalPixel = alpha_blend(GetPixel(x, y), color);
    if (AllowAlpha == false) {
        FinalPixel = color;
    }
    *((volatile uint32_t*)_GraphicsHandle.FbPtr + (y * (_GraphicsHandle.Pitch / 4) + x)) = 0xFFFFFFFF;
}

void GraphicsLibrary::aglib_draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color) {
    // Calculate the differences between the points
    int dx = aglib_abs(x2 - x1);
    int dy = aglib_abs(y2 - y1);

    int sx = (x1 < x2) ? 1 : -1;  // Step direction for x
    int sy = (y1 < y2) ? 1 : -1;  // Step direction for y

    int err = dx - dy;  // Initial error value

    int line_width_half = GraphicsHandleLineWidth / 2; // Half of the line width to extend in all directions

    while (1) {
        // Draw a rectangle/square around each point for the line width
        for (int i = -line_width_half; i <= line_width_half; ++i) {
            for (int j = -line_width_half; j <= line_width_half; ++j) {
                PutPixel(x1 + i, y1 + j, color);  // Plot the pixel at (x1, y1)
            }
        }

        // If we have reached the end point, break out of the loop
        if (x1 == x2 && y1 == y2) {
            break;
        }

        // Calculate the error and adjust the coordinates
        int e2 = err * 2;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void GraphicsLibrary::aglib_draw_dotted_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t dotlen, uint32_t color) {
    // Calculate the differences between the points
    int dx = aglib_abs(x2 - x1);
    int dy = aglib_abs(y2 - y1);

    int sx = (x1 < x2) ? 1 : -1;  // Step direction for x
    int sy = (y1 < y2) ? 1 : -1;  // Step direction for y

    int err = dx - dy;  // Initial error value

    int dot_counter = 0;  // Counter to control the dot intervals
    int line_width_half = GraphicsHandleLineWidth / 2; // Half of the line width

    while (1) {
        // Draw a dot every "dotlen" steps
        if (dot_counter == 0) {
            // Draw a rectangle/square around each dot for the line width
            for (int i = -line_width_half; i <= line_width_half; ++i) {
                for (int j = -line_width_half; j <= line_width_half; ++j) {
                    PutPixel(x1 + i, y1 + j, color); // Plot the pixel at (x1, y1)
                }
            }
        }

        // Increment dot counter and reset if we reach dotlen
        dot_counter = (dot_counter + 1) % dotlen;

        // If we have reached the end point, break out of the loop
        if (x1 == x2 && y1 == y2) {
            break;
        }

        // Calculate the error and adjust the coordinates
        int e2 = err * 2;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void GraphicsLibrary::aglib_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    for (uint32_t _y = y; _y <= y + height; ++_y) {
        for (uint32_t _x = x; _x <= x + width; ++_x) {
            // Draw border of the rectangle only
            if (_y == y || _y == y + height || _x == x || _x == x + width) {
                PutPixel(_x, _y, color);
            }
        }
    }
}

void GraphicsLibrary::aglib_draw_filled_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    for (uint32_t _y = y; _y <= y + height; ++_y) {
        for (uint32_t _x = x; _x <= x + width; ++_x) {
            PutPixel(_x, _y, color);  // Fill the rectangle
        }
    }
}

int GraphicsLibrary::aglib_abs(int x) {
    return (x < 0) ? -x : x;
}

void GraphicsLibrary::aglib_draw_circle(uint32_t x, uint32_t y, uint32_t radius, uint32_t color) {
    int dx = radius - 1;  // Start from the top-right of the circle
    int dy = 0;
    int err = dx - (radius * 2);  // Initial error value (to be updated in the loop)

    while (dx >= dy) {
        // Plot the 8 symmetric points around the circle
        PutPixel(x + dx, y - dy, color);  // Top-right
        PutPixel(x + dx, y + dy, color);  // Bottom-right
        PutPixel(x - dx, y - dy, color);  // Top-left
        PutPixel(x - dx, y + dy, color);  // Bottom-left
        PutPixel(x + dy, y - dx, color);  // Right-top
        PutPixel(x + dy, y + dx, color);  // Right-bottom
        PutPixel(x - dy, y - dx, color);  // Left-top
        PutPixel(x - dy, y + dx, color);  // Left-bottom

        // Adjust the error term for the next pixel
        if (err <= 0) {
            dy += 1;
            err += 2 * dy + 1;
        }
        if (err > 0) {
            dx -= 1;
            err -= 2 * dx + 1;
        }
    }
}

void GraphicsLibrary::aglib_draw_filled_circle(uint32_t x, uint32_t y, uint32_t radius, uint32_t color) {
    int dx = radius - 1;  // Start from the top-right of the circle
    int dy = 0;
    int err = dx - (radius * 2);  // Initial error value (to be updated in the loop)

    while (dx >= dy) {
        // For each y value, draw horizontal lines from (x-dx, y-dy) to (x+dx, y+dy)
        for (int i = x - dx; i <= x + dx; i++) {
            PutPixel(i, y - dy, color);  // Top horizontal line
            PutPixel(i, y + dy, color);  // Bottom horizontal line
        }

        // Repeat for the other 6 symmetric points
        for (int i = x - dy; i <= x + dy; i++) {
            PutPixel(i, y - dx, color);  // Left vertical line
            PutPixel(i, y + dx, color);  // Right vertical line
        }

        // Adjust the error term for the next pixel
        if (err <= 0) {
            dy += 1;
            err += 2 * dy + 1;
        }
        if (err > 0) {
            dx -= 1;
            err -= 2 * dx + 1;
        }
    }
}

void GraphicsLibrary::aglib_draw_text(uint32_t x, uint32_t y, const char* text, uint8_t* font, uint32_t color) {
    // Iterate over each character in the string
    for (size_t i = 0; text[i] != '\0'; ++i) {
        char c = text[i];

        // Each character is represented by 8 bytes in the font array
        const uint8_t* glyph = &font[c * 8];  // Assuming 8x8 font, each char is 8 bytes

        // Loop through each row of the 8x8 font
        for (size_t yy = 0; yy < 8; ++yy) {
            for (size_t xx = 0; xx < 8; ++xx) {
                // If the bit at the position is set, draw the pixel
                if (glyph[yy] & (1 << (7 - xx))) {  // Bit 0 is the leftmost bit
                    PutPixel(x + i * 8 + xx, y + yy, color);  // Position the character at (x, y)
                }
            }
        }
    }
}

void GraphicsLibrary::aglib_data_set_line_width(int width) {
    GraphicsHandleLineWidth = width;
}

void GraphicsLibrary::aglib_data_line_width_reset() {
    GraphicsHandleLineWidth = 1;
}

/*// FUTURE
aglib_load_image(data);
aglib_screen_shot(x, y, width, height, databuffer);
aglib_resize_image(image, width, height);
aglib_rotate_image(image, angle);
aglib_flip_image(image, fliphorizontal?);

aglib_create_window(width, height, title);
aglib_get_window_width(Window_t);
aglib_get_window_height(Window_t);
aglib_set_window_title(title);
aglib_set_window_fullscreen);

aglib_get_mouse_position(x, y);
aglib_is_mouse_button_pressed(buttoni);

aglib_is_key_pressed(scancode);
aglib_get_key_state(scancode);

aglib_check_collision(rect_t rect_a, rect_t rect_b);
*/// FUTURE END

//#if defined(__AGLIB_DEBUG__)
//aglib_enable_debug_mode()
//#endif

void GlobPutPixel(uint32_t x, uint32_t y, uint32_t color) {
    static_cast<volatile uint32_t*>(FbPtr_)[y * pitch/4 + x] = color;
}