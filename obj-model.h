#ifndef __OBJ_MODEL_H__
#define __OBJ_MODEL_H__

#include "geometry.h"
#include "types.h"
#include <vector>

bool loadOBJ(const char *path, std::vector<std::vector<int>> &out_faces,
             std::vector<Vec3f> &out_vertices, std::vector<Vec3f> &out_uvs,
             std::vector<Vec3f> &out_normals) {

  printf("Loading OBJ file %s...\n", path);

  FILE *file = fopen(path, "r");
  if (file == NULL) {
    printf("Impossible to open the file ! Are you in the right path ? See "
           "Tutorial 1 for details\n");
    getchar();
    return false;
  }

  char *line = NULL;
  ssize_t read;
  size_t len = 0;

  while ((read = getline(&line, &len, file)) != -1) {

    if (len == 0) {
      continue;
    }

    char dataType = line[0];

    if (dataType == 'v') {
      char secondChar = line[1];
      if (secondChar == ' ') {
        Vec3f vertex;
        sscanf(line, "v %f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
        out_vertices.push_back(vertex);

      } else if (secondChar == 't') {
        Vec3f uv;
        sscanf(line, "vt %f %f\n", &uv.x, &uv.y);
        uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture,
                      // which are inverted. Remove if you want to use TGA or
                      // BMP loaders.
        out_uvs.push_back(uv);

      } else if (secondChar == 'n') {
        Vec3f normal;
        sscanf(line, "n %f %f %f\n", &normal.x, &normal.y, &normal.z);
        out_normals.push_back(normal);
      }

    } else if (dataType == 'f') {
      std::vector<int> f;
      unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
      int matches = sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                           &vertexIndex[0], &uvIndex[0], &normalIndex[0],
                           &vertexIndex[1], &uvIndex[1], &normalIndex[1],
                           &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
      if (matches != 9) {
        matches = sscanf(line, "f %d//%d %d//%d %d//%d\n", &vertexIndex[0],
                         &normalIndex[0], &vertexIndex[1], &normalIndex[1],
                         &vertexIndex[2], &normalIndex[2]);
        if (matches != 6) {
          printf("File can't be read by our simple parser. Try exporting with "
                 "other options\n");
          fclose(file);
          return false;
        }
      }
      float x = --vertexIndex[0]; // indices start with 1
      float y = --vertexIndex[1];
      float z = --vertexIndex[2];

      f.push_back(x);
      f.push_back(y);
      f.push_back(z);
      out_faces.push_back(f);
    }
  }

  fclose(file);

  print("data loaded successfully");
  return true;
}

#endif //__OBJ_MODEL_H__
