
#ifndef __RENDER_H__
#define __RENDER_H__

#include <iostream>

#include "app.h"
#include "debug.h"
#include "image.h"
#include "types.h"
#include <GLFW/glfw3.h>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>

void drawAxis(glm::mat4 view, glm::mat4 perspective, glm::vec4 viewport, Image image) {
    glm::vec3 origin(0, 0, 0);
    glm::vec3 projectedOrigin = glm::project(origin, view, perspective, viewport);
    glm::vec3 xAxis = glm::vec3(1, 0, 0);
    glm::vec3 yAxis = glm::vec3(0, 1, 0);
    glm::vec3 zAxis = glm::vec3(0, 0, 1);

    glm::vec3 projectedxAxis = glm::project(xAxis, view, perspective, viewport);
    glm::vec3 projectedyAxis = glm::project(yAxis, view, perspective, viewport);
    glm::vec3 projectedzAxis = glm::project(zAxis, view, perspective, viewport);

    drawLine(projectedOrigin, projectedxAxis, image, RED);
    drawLine(projectedOrigin, projectedyAxis, image, GREEN);
    drawLine(projectedOrigin, projectedzAxis, image, BLUE);
}

void renderShape(Node *node, glm::mat4 perspective, glm::mat4 view, glm::vec4 viewport, App *app) {
    float near_plane = 1.0f;
    float far_plane = 10000.0f;
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    glm::mat4 lightView =
        glm::lookAt(app->lightDir, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    glm::mat4 parentWorldMatrix = glm::mat4(1);
    Node *parentNode = node->parent;
    if (!strcmp(parentNode->type, "transform")) {
        Transform transform = *(Transform *)parentNode;
        parentWorldMatrix = transform.matrix;
    }

    Shape modelData = *(Shape *)node;
    Png diffuseTexture = app->diffuseTexture;
    Png normalMapTexture = app->normalMapTexture;
    for (int i = 0; i < modelData.faces.size(); i++) {
        Face face = modelData.faces[i];
        glm::vec3 screenCoords[3];
        glm::vec3 lightSpaceCoords[3];

        glm::mat4 rotation = glm::rotate(glm::radians(app->rotateY), glm::vec3(0, 1, 0));
        glm::mat4 scale = glm::scale(glm::vec3(app->scale, app->scale, app->scale));
        glm::mat4 translation = glm::translate(glm::vec3(app->translateX, app->translateY, 0));

        glm::mat4 model = rotation;
        glm::mat4 modelView = view * parentWorldMatrix * model;
        glm::mat4 lightModelView = lightSpaceMatrix * parentWorldMatrix * model;

        glm::vec3 transformedVertices[3];
        glm::vec3 transformedNormals[3];

        for (int j = 0; j < 3; j++) {
            glm::vec3 v0 = face.verts[j];
            glm::vec4 vertex = glm::vec4(v0.x, v0.y, v0.z, 1);
            glm::vec4 transformedVertex = perspective * modelView * vertex;
            transformedVertices[j] = transformedVertex;

            glm::vec3 n0 = face.normals[j];
            glm::vec4 normal = glm::vec4(n0.x, n0.y, n0.z, 1);
            glm::mat4 normalModelMatrix = glm::transpose(glm::inverse(modelView));
            glm::vec3 transformedNormal = glm::normalize(glm::vec3(normalModelMatrix * normal));
            transformedNormals[j] = transformedNormal;

            screenCoords[j] = glm::project(glm::vec3(vertex), modelView, perspective, viewport);
            lightSpaceCoords[j] =
                glm::project(glm::vec3(vertex), lightModelView, perspective, viewport);
        }

        glm::vec3 normalA = face.normals[0];

        /* app->lightDir.y = cos(2 * glfwGetTime()); */
        /* app->lightDir.x = sin(2 * glfwGetTime()); */

        u32 normalColor = int((normalA.x + 1) / 2 * 255) | int((normalA.y + 1) / 2 * 255) << 8 |
                          int((normalA.z + 1) / 2 * 255) << 16 | (0 << 24);

        switch (app->renderMode) {

        case ZBUFFER:
        case SHADOWBUFFER:
        case TRIANGLES: {

            if (true) {
                drawShadowbuffer(lightSpaceCoords[0], lightSpaceCoords[1], lightSpaceCoords[2],
                                 app->image);

                drawTriangleWithTexture(
                    screenCoords[0], screenCoords[1], screenCoords[2], app->image, diffuseTexture,
                    normalMapTexture, face, transformedNormals, glm::mat3(view) * app->lightDir,
                    model, view, perspective, viewport, lightModelView);

            } else {
                float intensity =
                    glm::dot(glm::vec3(transformedNormals[0]), glm::normalize(app->lightDir));
                if (intensity < 0)
                    intensity = 0;

                u32 color = int(255 * intensity) | int(255 * intensity) << 8 |
                            int(255 * intensity) << 16 | (0 << 24);
                drawTriangle(screenCoords[0], screenCoords[1], screenCoords[2], app->image, color);
            }
            break;
        }

        case POINTS:
            for (int i = 0; i < 3; i++) {
                glm::vec2 coords = glm::ivec2(screenCoords[i]);
                drawPixel(coords.x, coords.y, normalColor, app->image);
            }
            break;

        case NORMALS:
            glm::vec3 normalEnd = face.verts[0] + normalA * app->normalLength;
            glm::vec3 screenCoordsEnd =
                glm::project(glm::vec3(normalEnd), modelView, perspective, viewport);
            drawLine(screenCoords[0], screenCoordsEnd, app->image, normalColor);
            break;
        }
    }
}

void renderWorld_r(Node *root, App *app, glm::mat4 perspective, glm::mat4 view,
                   glm::vec4 viewport) {
    std::vector<Node *> children = root->children;
    for (int i = 0; i < children.size(); i++) {
        Node *child = children[i];
        if (!strcmp(child->type, "shape")) {
            renderShape(child, perspective, view, viewport, app);
        }
        renderWorld_r(child, app, perspective, view, viewport);
    }
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

    glm::mat4 view = glm::lookAt(cam.pos, cam.target, cam.up);
    float zNear = 0.01f;
    float zFar = 1000.0f;
    glm::mat4 perspective = glm::perspectiveLH(cam.fov, width / float(height), 0.01f, 1000.0f);
    glm::vec4 viewport(0.0f, 0.0f, width - 1, height - 1);

    if (app->showAxis) {
        drawAxis(view, perspective, viewport, image);
    }

    Node *root = app->world->worldRoot;
    renderWorld_r(root, app, perspective, view, viewport);
}

#endif // __TYPES_H__
