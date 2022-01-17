#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "types.h"
#include <glm/gtx/transform.hpp>
#include <stdlib.h>

u32 RED = 255 | (0 << 8) | (0 << 16) | (0 << 24);
u32 GREEN = 0 | (255 << 8) | (0 << 16) | (0 << 24);
u32 BLUE = 0 | (0 << 8) | (255 << 16) | (0 << 24);
u32 WHITE = 255 | (255 << 8) | (255 << 16) | (255 << 24);

void drawPixel(u32 x, u32 y, u32 color, Image &image) {
    if (x > 0 && x <= image.width - 1 && y > 0 && y <= image.height - 1) {
        image.buffer[x + image.width * y] = color;
    }
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

/* void drawTriangle(glm::vec2 t0, glm::vec2 t1, glm::vec2 t2, Image &image, u32 color) { */
/*     int minX = imin(imin(imin(t0.x, t1.x), t2.x), image.width - 1); */
/*     int maxX = imax(imax(imax(t0.x, t1.x), t2.x), 0); */

/*     int minY = imin(imin(imin(t0.y, t1.y), t2.y), image.height - 1); */
/*     int maxY = imax(imax(imax(t0.y, t1.y), t2.y), 0); */

/*     for (int x = minX; x < maxX; x++) { */
/*         for (int y = minY; y < maxY; y++) { */

/*             glm::vec2 p(x, y); */
/*             bool insideEdgeA = edgeFunction(t1, t2, p); */
/*             bool insideEdgeB = edgeFunction(t2, t0, p); */
/*             bool insideEdgeC = edgeFunction(t0, t1, p); */

/*             if (insideEdgeA && insideEdgeB && insideEdgeC) { */
/*                 drawPixel(p.x, p.y, color, image); */
/*             } */
/*         } */
/*     } */
/* } */

unsigned char *samplePngTexture(Png png, float u, float v) {
    u32 width = png.width;
    u32 height = png.height;
    u32 x = width * u;
    u32 y = height * v;

    u32 offset = (y * width + x) * 4;
    return &png.image[offset];
}

float linearToSrgb(float theLinearValue) {
    return theLinearValue <= 0.0031308f ? theLinearValue * 12.92f
                                        : powf(theLinearValue, 1.0f / 2.4f) * 1.055f - 0.055f;
}

glm::vec3 toBarycentric(glm::vec3 bc, glm::vec3 *v) {
    return bc[0] * v[0] + bc[1] * v[1] + bc[2] * v[2];
}

void calcTangentSpace2(Face &face) {
    // tangents
    glm::vec3 v0 = face.verts[0];
    glm::vec3 v1 = face.verts[1];
    glm::vec3 v2 = face.verts[2];

    glm::vec3 uv0 = face.uvs[0];
    glm::vec3 uv1 = face.uvs[1];
    glm::vec3 uv2 = face.uvs[2];

    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;

    glm::vec3 deltaUV1 = uv1 - uv0;
    glm::vec3 deltaUV2 = uv2 - uv0;

    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    glm::vec3 tangent, bitangent;
    tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent.t = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
    face.tangent = glm::normalize(tangent);

    bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    bitangent.t = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
    face.bitangent = glm::normalize(bitangent);

    face.faceNormal = glm::cross(edge1, edge2);
}

void drawTriangleWithTexture(glm::vec3 t0, glm::vec3 t1, glm::vec3 t2, Image &image,
                             Png diffuseTexture, Png normalMapTexture, Face face,
                             glm::vec3 *normals, glm::vec3 lightDir, glm::mat4 model,
                             glm::mat4 view, glm::mat4 perspective, glm::vec4 viewport) {
    int minX = imin(imin(imin(t0.x, t1.x), t2.x), image.width - 1);
    int maxX = imax(imax(imax(t0.x, t1.x), t2.x), 0);

    int minY = imin(imin(imin(t0.y, t1.y), t2.y), image.height - 1);
    int maxY = imax(imax(imax(t0.y, t1.y), t2.y), 0);

    glm::vec3 P;
    u32 width = diffuseTexture.width;
    u32 height = diffuseTexture.height;

    for (P.x = minX; P.x <= maxX; P.x++) {
        for (P.y = minY; P.y <= maxY; P.y++) {

            glm::vec3 bc = barycentric(t0, t1, t2, P);

            if (bc.x < 0 || bc.y < 0 || bc.z < 0)
                continue;

            P.z = 0;
            P.z += t0.z * bc[0];
            P.z += t1.z * bc[1];
            P.z += t2.z * bc[2];

            glm::mat3 modelVector = glm::transpose(glm::inverse(glm::mat3(view * model)));

            glm::vec3 uv = toBarycentric(bc, face.uvs);
            glm::vec3 bc_normal = toBarycentric(bc, normals);

            glm::mat3 tangentSpace;
            glm::vec3 edge1 = t1 - t0;
            glm::vec3 edge2 = t2 - t0;

            glm::vec3 T = glm::normalize(modelVector * face.tangent);
            glm::vec3 B = glm::normalize(modelVector * face.bitangent);
            glm::vec3 N = glm::normalize(bc_normal);

            T = glm::normalize(T - N * glm::dot(N, T));
            B = glm::normalize(B - N * glm::dot(N, B));

            if (glm::dot(glm::cross(T, N), B) < 0.0f){
                T = T * -1.0f;
            }

            tangentSpace[0] = T;
            tangentSpace[1] = B;
            tangentSpace[2] = N;

            u32 x = width * uv.x;
            u32 y = height * uv.y;

            u32 offset = (y * width + x) * 4;

            unsigned char nr = normalMapTexture.image[offset];
            unsigned char ng = normalMapTexture.image[offset + 1];
            unsigned char nb = normalMapTexture.image[offset + 2];

            float fr = nr / 255.0f;
            float fg = ng / 255.0f;
            float fb = nb / 255.0f;

            glm::vec3 textureNormal =
                glm::vec3(2.0f * fr - 1.0f, 2.0f * fg - 1.0f, 2.0f * fb - 1.0f);

            glm::vec3 normal = glm::normalize(tangentSpace * textureNormal);
            /* normal = bc_normal; */

            float intensity = glm::dot(normal, glm::normalize(lightDir));
            if (intensity < 0) {
                intensity = 0;
            } else {
                intensity = pow(intensity, 2);
            }

            u8 r = diffuseTexture.image[offset] * intensity;
            u8 g = diffuseTexture.image[offset + 1] * intensity;
            u8 b = diffuseTexture.image[offset + 2] * intensity;
            u8 rsrgb = u8(linearToSrgb(r / 255.0f) * 255);
            u8 gsrgb = u8(linearToSrgb(g / 255.0f) * 255);
            u8 bsrgb = u8(linearToSrgb(b / 255.0f) * 255);
            u32 color = rsrgb | gsrgb << 8 | bsrgb << 16 | (255 << 24);

            u32 colorNormal = int(((normal.x + 1.0f) / 2.0f) * 255.0f) |
                              int(((normal.y + 1.0f) / 2.0f) * 255.0f) << 8 |
                              int(((normal.z + 1.0f) / 2.0f) * 255) << 16 | (255 << 24);

            /* colorNormal = 0 | 0 | */
            /*                   int(((normal.z + 1.0f) / 2.0f) * 255) << 16 | (255 << 24); */

            u32 colorNormalTexture = nr | ng << 8 | nb << 16 | (255 << 24);

            int coord = int(P.x + P.y * image.width);
            if (image.zbuffer[coord] < P.z) {
                image.zbuffer[coord] = P.z;
                drawPixel(P.x, P.y, color, image);
            }
        }
    }
}

void drawTriangle(glm::vec3 t0, glm::vec3 t1, glm::vec3 t2, Image &image, u32 color) {
    int minX = imin(imin(imin(t0.x, t1.x), t2.x), image.width - 1);
    int maxX = imax(imax(imax(t0.x, t1.x), t2.x), 0);

    int minY = imin(imin(imin(t0.y, t1.y), t2.y), image.height - 1);
    int maxY = imax(imax(imax(t0.y, t1.y), t2.y), 0);

    glm::vec3 P;
    for (P.x = minX; P.x <= maxX; P.x++) {
        for (P.y = minY; P.y <= maxY; P.y++) {

            glm::vec3 bc_screen = barycentric(t0, t1, t2, P);

            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
                continue;

            P.z = 0;
            P.z += t0.z * bc_screen[0];
            P.z += t1.z * bc_screen[1];
            P.z += t2.z * bc_screen[2];

            int coord = int(P.x + P.y * image.width);

            if (image.zbuffer[coord] < P.z) {
                image.zbuffer[coord] = P.z;

                drawPixel(P.x, P.y, color, image);
            }
        }
    }
}

void drawCircle(int xc, int yc, int x, int y, Image &image, u32 color) {
    drawPixel(xc + x, yc + y, color, image);
    drawPixel(xc - x, yc + y, color, image);
    drawPixel(xc + x, yc - y, color, image);
    drawPixel(xc - x, yc - y, color, image);
    drawPixel(xc + y, yc + x, color, image);
    drawPixel(xc - y, yc + x, color, image);
    drawPixel(xc + y, yc - x, color, image);
    drawPixel(xc - y, yc - x, color, image);
}

void circleBres(int xc, int yc, int r, Image &image, u32 color) {
    int x = 0, y = r;
    int d = 3 - 2 * r;
    drawCircle(xc, yc, x, y, image, color);
    while (y >= x) {
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else
            d = d + 4 * x + 6;
        drawCircle(xc, yc, x, y, image, color);
    }
}

#endif // __IMAGE_H_
