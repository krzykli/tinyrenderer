#ifndef __OBJ_MODEL_H__
#define __OBJ_MODEL_H__

#include "geometry.h"
#include "types.h"
#include <vector>

bool loadOBJ(const char *path, std::vector<Vec3f> &out_vertices,
             std::vector<Vec3f> &out_uvs, std::vector<Vec3f> &out_normals) {

  printf("Loading OBJ file %s...\n", path);

  std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
  std::vector<Vec3f> temp_vertices;
  std::vector<Vec3f> temp_uvs;
  std::vector<Vec3f> temp_normals;

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
        temp_vertices.push_back(vertex);

      } else if (secondChar == 't') {
        Vec3f uv;
        sscanf(line, "vt %f %f\n", &uv.x, &uv.y);
        uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture,
                      // which are inverted. Remove if you want to use TGA or
                      // BMP loaders.
        temp_uvs.push_back(uv);

      } else if (secondChar == 'n') {
        Vec3f normal;
        sscanf(line, "n %f %f %f\n", &normal.x, &normal.y, &normal.z);
        temp_normals.push_back(normal);
      }
    } else if (dataType == 'f') {
      std::string vertex1, vertex2, vertex3;
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
      vertexIndices.push_back(vertexIndex[0]);
      vertexIndices.push_back(vertexIndex[1]);
      vertexIndices.push_back(vertexIndex[2]);

      if (matches == 9) {
        uvIndices.push_back(uvIndex[0]);
        uvIndices.push_back(uvIndex[1]);
        uvIndices.push_back(uvIndex[2]);
      }

      normalIndices.push_back(normalIndex[0]);
      normalIndices.push_back(normalIndex[1]);
      normalIndices.push_back(normalIndex[2]);
    }
  }

  // For each vertex of each triangle
  for (unsigned int i = 0; i < vertexIndices.size(); i++) {

    // Get the indices of its attributes
    unsigned int vertexIndex = vertexIndices[i];
    /* unsigned int uvIndex = uvIndices[i]; */
    unsigned int normalIndex = normalIndices[i];

    // Get the attributes thanks to the index
    Vec3f vertex = temp_vertices[vertexIndex - 1];
    /* Vec3f uv = temp_uvs[ uvIndex-1 ]; */
    Vec3f normal = temp_normals[normalIndex - 1];

    // Put the attributes in buffers
    out_vertices.push_back(vertex);
    /* out_uvs     .push_back(uv); */
    out_normals.push_back(normal);
  }
  fclose(file);

  print("data loaded successfully");
  return true;
}

#endif //__OBJ_MODEL_H__
