#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>


typedef struct Vec2f {
    float x;
    float y;
} Vec2f;

typedef union {
   float raw[3];
   struct { float x; float y; float z; };
} Vec3f;

typedef struct Vec2i {
   float raw[2];
   struct { int x; int y; };
} Vec2i;

typedef struct Vec3i {
    int x;
    int y;
    int z;
} Vec3i;

#endif //__GEOMETRY_H__t
