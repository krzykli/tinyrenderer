#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>
#include <vector>
#include <glm/gtx/transform.hpp>

#define persist static

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

enum RenderMode { TRIANGLES = 0, POINTS = 1, NORMALS = 2, ZBUFFER = 3 };

typedef struct Camera {
    glm::vec3 pos;
    glm::vec3 front;
    glm::vec3 up;
    float fov;

    float yaw;
    float pitch;
    float lastX;
    float lastY;
} Camera;

typedef struct Image {
    u32 *buffer;
    float *zbuffer;
    u32 width;
    u32 height;
} Image;

typedef struct {
  glm::vec3 verts[3];
  glm::vec3 normals[3];
  glm::vec3 uvs[3];
} Face;

typedef struct ModelData{
    std::vector<Face> faces;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> uvs;
    std::vector<glm::vec3> normals;
} ModelData;


typedef struct Png {
    unsigned char* image;
    unsigned width;
    unsigned height;
} Png;


#endif // __TYPES_H__
