
#ifndef __RENDER_H__
#define __RENDER_H__

#include <iostream>

#include "app.h"
#include "debug.h"
#include "image.h"
#include "types.h"
#include <GLFW/glfw3.h>
#include <glm/gtx/transform.hpp>

#include <glm/gtx/string_cast.hpp>

void drawAxis(glm::mat4 view, glm::mat4 perspective, glm::vec4 viewport, Image image) {
    glm::vec3 origin(0, 0, 0);
    glm::vec3 projectedOrigin = glm::project(origin, view, perspective, viewport);
    glm::vec3 xAxis = glm::vec3(1, 0, 0);
    glm::vec3 yAxis = glm::vec3(0, 1, 0);
    glm::vec3 zAxis = glm::vec3(0, 0, 1);

    glm::vec3 projectedxAxis = glm::project(xAxis, view, perspective, viewport);
    glm::vec3 projectedyAxis = glm::project(yAxis, view, perspective, viewport);
    glm::vec3 projectedzAxis = glm::project(zAxis, view, perspective, viewport);

    drawLine(glm::vec2(projectedOrigin), glm::vec2(projectedxAxis), image, RED);
    drawLine(glm::vec2(projectedOrigin), glm::vec2(projectedyAxis), image, GREEN);
    drawLine(glm::vec2(projectedOrigin), glm::vec2(projectedzAxis), image, BLUE);
}

void trRender(App *app) {
    if (app->turntable) {
        app->rotateY = app->rotateY + app->turntableSpeed * app->deltaTime * 20;
        if (app->rotateY > 360.f) {
            app->rotateY = 0;
        }
    }

    Image image = app->image;
    Camera cam = app->camera;
    int width = image.width;
    int height = image.height;

    glm::mat4 view = glm::lookAt(cam.pos, cam.pos + cam.front, glm::vec3(0, 1, 0));
    glm::mat4 perspective =
        glm::perspective(glm::radians(45.0f), width / float(height), 0.01f, 1000.0f);
    glm::vec4 viewport(0.0f, 0.0f, width - 1, height - 1);

    if (app->showAxis) {
        drawAxis(view, perspective, viewport, image);
    }

    for (int i = 0; i < app->modelData.faces.size(); i++) {
        Face f = app->modelData.faces[i];
        glm::vec3 screenCoords[3];

        glm::mat4 rotation = glm::rotate(glm::radians(app->rotateY), glm::vec3(0, 1, 0));
        glm::mat4 scale = glm::scale(glm::vec3(app->scale, app->scale, app->scale));
        glm::mat4 translation = glm::translate(glm::vec3(app->translateX, app->translateY, 0));

        /* glm::mat4 modelMatrix = translation * scale * rotation; */
        glm::mat4 modelView = view * rotation;

        glm::vec3 transformedVertices[3];

        for (int j = 0; j < 3; j++) {
            glm::vec3 v0 = f.verts[j];
            glm::vec4 vertex = glm::vec4(v0.x, v0.y, v0.z, 1);
            glm::vec4 transformedVertex = perspective * modelView * vertex;
            transformedVertices[j] = transformedVertex;
            screenCoords[j] = glm::project(glm::vec3(vertex), modelView, perspective, viewport);
        }

        glm::vec3 normalA = f.normals[0];
        glm::mat4 modelMatrix = translation * rotation;
        glm::mat4 normalModelMatrix = glm::transpose(glm::inverse(modelView));
        glm::vec4 normal = glm::vec4(normalA.x, normalA.y, normalA.z, 1);
        glm::vec3 transformedNormal = glm::normalize(glm::vec3(normalModelMatrix * normal));

        glm::vec3 lightDir(0, 1, -1);
        float intensity = glm::dot(glm::vec3(transformedNormal), glm::normalize(lightDir));
        if (intensity < 0)
            intensity = 0;

        u32 normalColor = int((normalA.x + 1) / 2 * 255) | int((normalA.y + 1) / 2 * 255) << 8 |
                          int((normalA.z + 1) / 2 * 255) << 16 | (0 << 24);

        switch (app->renderMode) {

        case TRIANGLES: {
            u32 color = int(255 * intensity) | int(255 * intensity) << 8 |
                        int(255 * intensity) << 16 | (0 << 24);
            drawTriangle(screenCoords[0], screenCoords[1], screenCoords[2], app->image, color);
            break;
        }

        case POINTS:
            for (int i = 0; i < 3; i++) {
                glm::vec2 coords = glm::ivec2(screenCoords[i]);
                drawPixel(coords.x, coords.y, normalColor, app->image);
            }
            break;

        case NORMALS:
            glm::vec3 normalEnd = f.verts[0] + normalA * app->normalLength;
            glm::vec3 screenCoordsEnd =
                glm::project(glm::vec3(normalEnd), modelView, perspective, viewport);
            drawLine(screenCoords[0], screenCoordsEnd, app->image, normalColor);
            break;
        }
    }
}

#endif // __TYPES_H__
