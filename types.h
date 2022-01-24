#ifndef __TYPES_H__
#define __TYPES_H__

#include <glm/gtx/transform.hpp>
#include <stdint.h>
#include <vector>

#define persist static

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

enum RenderMode { TRIANGLES = 0, POINTS = 1, NORMALS = 2, ZBUFFER = 3, SHADOWBUFFER = 4};

typedef struct Camera {
    glm::vec3 pos;
    glm::vec3 target;
    glm::vec3 up;
    float fov;

    float yaw;
    float pitch;
    float lastX;
    float lastY;
} Camera;

typedef struct Image {
    u32 *buffer;
    u32 *depth;
    u32 *glow;
    float *zbuffer;
    float *shadowbuffer;
    u32 width;
    u32 height;
} Image;

typedef struct {
    glm::vec3 verts[3];
    glm::vec3 normals[3];
    glm::vec3 uvs[3];
    glm::vec3 tangent;
    glm::vec3 bitangent;
    glm::vec3 faceNormal;
} Face;

typedef struct Png {
    unsigned char *buffer;
    unsigned width;
    unsigned height;
} Png;

typedef struct Node {
    struct Node* parent;
    struct std::vector<Node*> children;
    char const* name;
    char const* type;
} Node;

typedef struct World {
    Node* worldRoot;
} World;

typedef struct Transform {
    Node node;
    glm::mat4 matrix;
} Transform;

typedef struct Shape {
    Node node;

    std::vector<Face> faces;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> uvs;
    std::vector<glm::vec3> normals;
} Shape;

#endif // __TYPES_H__
