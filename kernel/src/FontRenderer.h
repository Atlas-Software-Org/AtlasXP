#ifndef D67DD864_78DA_43BE_AF8E_E43EB5B16348
#define D67DD864_78DA_43BE_AF8E_E43EB5B16348

#include <stdint.h>
#include <stddef.h>
#include <Atlasglib/aglib.h>
#include <Atlasglib/aglib_global.h>

void SetGfxFont(GraphicsLibrary gfx);
GraphicsLibrary GetGfxFont();

void font_char(char c, size_t x, size_t y, uint32_t color);
void font_str(const char *s, size_t x, size_t y, uint32_t color);

#endif /* D67DD864_78DA_43BE_AF8E_E43EB5B16348 */
