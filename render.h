
#ifndef __RENDER_H__
#define __RENDER_H__

#include "app.h"
#include "debug.h"
#include "image.h"
#include "types.h"
#include <glm/gtx/transform.hpp>

void trRender(App *app) {
    if (app->turntable) {
        app->rotateY = app->rotateY + app->turntableSpeed * app->deltaTime * 20;
        if (app->rotateY > 360.f) {
            app->rotateY = 0;
        }
    }

    for (int i = 0; i < app->modelData.faces.size(); i++) {
        Face f = app->modelData.faces[i];
        glm::vec2 screenCoords[3];

        glm::mat4 rotation = glm::rotate(glm::radians(app->rotateY), glm::vec3(0, 1, 0));
        glm::mat4 scale = glm::scale(glm::vec3(app->scale, app->scale, app->scale));
        glm::mat4 translation = glm::translate(glm::vec3(app->translateX, app->translateY, 0));

        glm::mat4 modelMatrix = translation * scale * rotation;

        glm::vec3 transformedVertices[3];

        for (int j = 0; j < 3; j++) {
            glm::vec3 v0 = f.verts[j];
            glm::vec4 vertex = glm::vec4(v0.x, v0.y, v0.z, 1);
            glm::vec4 transformedVertex = modelMatrix * vertex;
            transformedVertices[j] = transformedVertex;
            screenCoords[j] = glm::vec2((int)transformedVertex.x, (int)transformedVertex.y);
        }

        glm::vec3 normalA = f.normals[0];
        modelMatrix = translation * rotation;
        glm::mat4 normalModelMatrix = glm::transpose(glm::inverse(modelMatrix));
        glm::vec4 normal = glm::vec4(normalA.x, normalA.y, normalA.z, 1);
        glm::vec3 transformedNormal = glm::normalize(glm::vec3(normalModelMatrix * normal));

        glm::vec3 lightDir(0, 1, -1);
        float intensity = glm::dot(glm::vec3(transformedNormal), glm::normalize(lightDir));

        u32 normalColor = int((normalA.x + 1) / 2 * 255) | int((normalA.y + 1) / 2 * 255) << 8 |
                          int((normalA.z + 1) / 2 * 255) << 16 | (0 << 24);

        switch (app->renderMode) {

        case TRIANGLES: {
            u32 color = int(255 * intensity) | int(255 * intensity) << 8 |
                        int(255 * intensity) << 16 | (0 << 24);
            if (intensity > 0) {
                drawTriangle(screenCoords[0], screenCoords[1], screenCoords[2], app->image, color);
            }
            break;
        }

        case POINTS:

            for (int i = 0; i < 3; i++) {
                glm::vec2 coords = screenCoords[i];
                drawPixel(coords.x, coords.y, normalColor, app->image);
            }
            break;

        case NORMALS:
            glm::vec2 normalStart = glm::vec2(int(transformedVertices[0].x + transformedNormal.x),
                                              int(transformedVertices[0].y + transformedNormal.y));
            glm::vec2 normalEnd =
                glm::vec2(int(transformedVertices[0].x + transformedNormal.x * 10.0f),
                          int(transformedVertices[0].y + transformedNormal.y * 10.0f));
            drawLine(normalStart, normalEnd, app->image, normalColor);
            break;
        }
    }
}

#endif // __TYPES_H__
