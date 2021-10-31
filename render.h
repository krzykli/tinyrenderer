
#ifndef RENDERH
#define RENDERH

#include <stdlib.h>

typedef struct Image
{
    uint32_t* buffer;
    uint32_t width;
    uint32_t height;
} Image;


void drawPixel(uint32_t x, uint32_t y, uint32_t color, Image &image) {
    image.buffer[x + image.width * y] = color;
}


#endif // RENDERH
