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

enum RenderMode { TRIANGLES = 0, POINTS = 1, NORMALS = 2 };

typedef struct Image {
    u32 *buffer;
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

#endif // __TYPES_H__
