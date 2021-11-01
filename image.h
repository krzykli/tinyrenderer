#ifndef RENDERH
#define RENDERH

#include <stdlib.h>
#include "types.h"

typedef struct Image
{
    u32* buffer;
    u32 width;
    u32 height;
} Image;

void clearImage(Image &image) {
    for (int i=0; i < image.width * image.height; i++) {
        image.buffer[i] = 0;
    }
}

void drawPixel(u32 x, u32 y, u32 color, Image &image) {
    image.buffer[x + image.width * y] = color;
}


#endif // RENDERH
