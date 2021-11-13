#ifndef IMAGEH
#define IMAGEH

#include <stdlib.h>
#include "types.h"
#include "geometry.h"

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

void drawLine(Vec2i v0, Vec2i v1, Image &image, u32 color) {

    int x0 = v0.x;
    int y0 = v0.y;

    int x1 = v1.x;
    int y1 = v1.y;

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


int imin(int a, int b)
{
  return (a < b) ? a : b;
}

int imax(int a, int b)
{
  return (a > b) ? a : b;
}

bool edgeFunction(Vec2i &a, Vec2i &b, Vec2i &c)
{
    return ((c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x) >= 0);
}

void drawTriangle(Vec2i t0, Vec2i t1, Vec2i t2, Image &image, u32 color) {
    int minX = imin(imin(t0.x, t1.x), t2.x);
    int maxX = imax(imax(t0.x, t1.x), t2.x);

    int minY = imin(imin(t0.y, t1.y), t2.y);
    int maxY = imax(imax(t0.y, t1.y), t2.y);

    for (int x = minX; x < maxX; x++) {
        for (int y = minY; y < maxY; y++) {

            Vec2i p = { .x = x, .y = y };
            bool insideEdgeA = edgeFunction(t1, t2, p);
            bool insideEdgeB = edgeFunction(t2, t0, p);
            bool insideEdgeC = edgeFunction(t0, t1, p);

            if (insideEdgeA && insideEdgeB && insideEdgeC) {
                drawPixel(p.x, p.y, color, image);
            }
        }
    }
}

#endif // IMAGEH
