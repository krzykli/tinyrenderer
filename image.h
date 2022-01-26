#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "types.h"
#include <glm/gtx/transform.hpp>
#include <stdlib.h>

u32 RED = 255 | (0 << 8) | (0 << 16) | (0 << 24);
u32 GREEN = 0 | (255 << 8) | (0 << 16) | (0 << 24);
u32 BLUE = 0 | (0 << 8) | (255 << 16) | (0 << 24);
u32 WHITE = 255 | (255 << 8) | (255 << 16) | (255 << 24);

inline bool boundsCheck(u32 x, u32 y, u32 width, u32 height) {
    return (x > 0 && x <= width - 1 && y > 0 && y <= height - 1);
}

void drawPixel(u32 x, u32 y, u32 color, Image &image) {
    if (x > 0 && x <= image.width - 1 && y > 0 && y <= image.height - 1) {
        image.buffer[x + image.width * y] = color;
    }
}

void printVec3(const char *label, glm::vec3 vector) {
    printf("%s x:%f y:%f z:%f\n", label, vector.x, vector.y, vector.z);
}

inline u32 rgbToU32(u8 r, u8 g, u8 b) { return r | g << 8 | b << 16 | (255 << 24); }

inline u32 vec3ToU32(glm::vec3 v) {
    return int(v.x) | int(v.y) << 8 | int(v.z) << 16 | (255 << 24);
}

glm::vec3 barycentric(glm::vec3 A, glm::vec3 B, glm::vec3 C, glm::vec3 P) {
    glm::vec3 s[2];
    for (int i = 2; i--;) {
        s[i][0] = C[i] - A[i];
        s[i][1] = B[i] - A[i];
        s[i][2] = A[i] - P[i];
    }
    glm::vec3 u = cross(s[0], s[1]);
    if (std::abs(u[2]) > 1e-2)
        return glm::vec3(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    return glm::vec3(-1, 1, 1);
}

void drawLine(glm::vec3 v0, glm::vec3 v1, Image &image, u32 color) {

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

int imin(int a, int b) { return (a < b) ? a : b; }

int imax(int a, int b) { return (a > b) ? a : b; }

bool edgeFunction(glm::vec2 &a, glm::vec2 &b, glm::vec2 &c) {
    return ((c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x) >= 0);
}

inline float linearToSrgb(float theLinearValue) {
    return theLinearValue <= 0.0031308f ? theLinearValue * 12.92f
                                        : powf(theLinearValue, 1.0f / 2.4f) * 1.055f - 0.055f;
}

glm::vec3 toBarycentric(glm::vec3 bc, glm::vec3 *v) {
    return bc[0] * v[0] + bc[1] * v[1] + bc[2] * v[2];
}

glm::vec3 sampleNormalTexture(int offset, Png texture) {
    unsigned char r = texture.buffer[offset];
    unsigned char g = texture.buffer[offset + 1];
    unsigned char b = texture.buffer[offset + 2];

    float fr = r / 255.0f;
    float fg = g / 255.0f;
    float fb = b / 255.0f;

    glm::vec3 textureNormal = glm::vec3(2.0f * fr - 1.0f, 2.0f * fg - 1.0f, 2.0f * fb - 1.0f);

    return textureNormal;
}

glm::vec3 sampleTexture(int offset, Png texture) {
    unsigned char r = texture.buffer[offset];
    unsigned char g = texture.buffer[offset + 1];
    unsigned char b = texture.buffer[offset + 2];

    return glm::vec3(r, g, b);
}

#endif // __IMAGE_H_
