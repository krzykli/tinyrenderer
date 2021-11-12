#ifndef IMAGEH
#define IMAGEH

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
    if (x > 0 && x < image.width - 1 && y > 0 && y < image.height - 1) {
        image.buffer[x + image.width * y] = color;
    }
}

void drawLine(int x0, int y0, int x1, int y1, Image &image, u32 color) {

    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }

    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int dx = x1 - x0;
    int dy = y1 - y0;

    int derror2 = std::abs(dy) * 2;
    int error2 = 0;
    int y = y0;

    for (int x = x0; x <= x1; x++) {
        if (steep) {
            drawPixel(y, x, color, image);
        } else {
            drawPixel(x, y, color, image);
        }

        error2 += derror2;
        if (error2 > .5) {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx * 2;
        }
    }
}


#endif // IMAGEH
