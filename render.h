
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

void drawAxis(glm::mat4 view, glm::mat4 projection, glm::vec4 viewport, Image image) {
    glm::vec3 origin(0, 0, 0);
    glm::vec3 projectedOrigin = glm::project(origin, view, projection, viewport);
    glm::vec3 xAxis = glm::vec3(1, 0, 0);
    glm::vec3 yAxis = glm::vec3(0, 1, 0);
    glm::vec3 zAxis = glm::vec3(0, 0, 1);

    glm::vec3 projectedxAxis = glm::project(xAxis, view, projection, viewport);
    glm::vec3 projectedyAxis = glm::project(yAxis, view, projection, viewport);
    glm::vec3 projectedzAxis = glm::project(zAxis, view, projection, viewport);

    drawLine(projectedOrigin, projectedxAxis, image, RED);
    drawLine(projectedOrigin, projectedyAxis, image, GREEN);
    drawLine(projectedOrigin, projectedzAxis, image, BLUE);
}

glm::mat4 getParentMatrix(Node *node) {
    glm::mat4 matrix = glm::mat4(1);
    Node *parentNode = node->parent;
    if (!strcmp(parentNode->type, "transform")) {
        Transform transform = *(Transform *)parentNode;
        matrix = transform.matrix;
    }
    return matrix;
}

inline glm::mat4 getLightView(glm::vec3 lightDir) {
    return glm::lookAt(lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 getLightProjection() {
    float nearPlane = 0.01f;
    float farPlane = 10000.0f;
    return glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, nearPlane, farPlane);
}


typedef struct DefaultVertexShaderOut {
    glm::vec3 position[3];
    glm::vec3 normals[3];
} DefaultVertexShaderOut;

typedef struct DefaultVertexShaderIn {
    Face face;
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec4 viewport;
} DefaultVertexShaderIn;

typedef struct UberFragmentShaderIn {
    glm::vec3 *position;
    Face face;

    u32 *buffer;
    u32 bufferWidth;
    u32 bufferHeight;

    glm::vec3 lightDir;
    Png diffuseTexture;
    Png normalMapTexture;
    Png specTexture;
    Png glowTexture;

    glm::vec3 camPos;
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec4 viewport;

    Image image;
} UberFragmentShaderIn;

typedef struct UberFragmentShaderOut {
    glm::vec2 screenCoords;
    glm::vec3 color;
} UberFragmentShaderOut;

DefaultVertexShaderOut runDefaultVertexProgram(DefaultVertexShaderIn in) {
    glm::mat4 modelView = in.view * in.model;

    DefaultVertexShaderOut vertexOut;

    for (int i = 0; i < 3; i++) {
        glm::vec3 v0 = in.face.verts[i];
        glm::vec4 vertex = glm::vec4(v0.x, v0.y, v0.z, 1);

        glm::vec3 n0 = in.face.normals[i];
        glm::vec4 normal = glm::vec4(n0.x, n0.y, n0.z, 1);
        glm::mat4 normalModelMatrix = glm::transpose(glm::inverse(modelView));
        glm::vec3 transformedNormal = glm::normalize(glm::vec3(normalModelMatrix * normal));
        vertexOut.normals[i] = transformedNormal;

        vertexOut.position[i] =
            glm::project(glm::vec3(vertex), modelView, in.projection, in.viewport);
    }

    return vertexOut;
}

glm::vec3 computeBarycentricCoords(glm::vec3 A, glm::vec3 B, glm::vec3 C, glm::vec3 P) {
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

void runShadowFragmentProgram(UberFragmentShaderIn in) {
    glm::vec3 p0 = in.position[0];
    glm::vec3 p1 = in.position[1];
    glm::vec3 p2 = in.position[2];

    int minX = imin(imin(imin(p0.x, p1.x), p2.x), in.bufferWidth - 1);
    int maxX = imax(imax(imax(p0.x, p1.x), p2.x), 0);

    int minY = imin(imin(imin(p0.y, p1.y), p2.y), in.bufferHeight - 1);
    int maxY = imax(imax(imax(p0.y, p1.y), p2.y), 0);

    glm::vec3 frag;

    for (frag.x = minX; frag.x <= maxX; frag.x++) {
        for (frag.y = minY; frag.y <= maxY; frag.y++) {

            glm::vec3 barCoords = computeBarycentricCoords(p0, p1, p2, frag);

            if (barCoords.x < 0 || barCoords.y < 0 || barCoords.z < 0)
                continue;

            frag.z = 0;
            frag.z += p0.z * barCoords[0];
            frag.z += p1.z * barCoords[1];
            frag.z += p2.z * barCoords[2];

            int coord = int(frag.x + frag.y * in.image.width);
            if (in.image.shadowbuffer[coord] < frag.z) {
                in.image.shadowbuffer[coord] = frag.z;
            }
        }
    }
}


void runUberFragmentProgram(UberFragmentShaderIn in) {
    glm::vec3 p0 = in.position[0];
    glm::vec3 p1 = in.position[1];
    glm::vec3 p2 = in.position[2];

    int minX = imin(imin(imin(p0.x, p1.x), p2.x), in.bufferWidth - 1);
    int maxX = imax(imax(imax(p0.x, p1.x), p2.x), 0);

    int minY = imin(imin(imin(p0.y, p1.y), p2.y), in.bufferHeight - 1);
    int maxY = imax(imax(imax(p0.y, p1.y), p2.y), 0);

    glm::vec3 frag;

    glm::mat4 modelView = in.view * in.model;

    glm::mat4 lightView = getLightView(in.lightDir);
    glm::mat4 lightModelView = lightView * in.model;

    u32 textureWidth = in.diffuseTexture.width; // all of them are the same
    u32 textureHeight = in.diffuseTexture.height;

    for (frag.x = minX; frag.x <= maxX; frag.x++) {
        for (frag.y = minY; frag.y <= maxY; frag.y++) {

            glm::vec3 barCoords = computeBarycentricCoords(p0, p1, p2, frag);

            if (barCoords.x < 0 || barCoords.y < 0 || barCoords.z < 0)
                continue;

            frag.z = 0;
            frag.z += p0.z * barCoords[0];
            frag.z += p1.z * barCoords[1];
            frag.z += p2.z * barCoords[2];

            glm::mat3 modelVector = glm::transpose(glm::inverse(glm::mat3(modelView)));

            glm::vec3 uv = toBarycentric(barCoords, in.face.uvs);
            glm::vec3 barycentricNormal = toBarycentric(barCoords, in.face.normals);

            glm::mat3 tangentSpace;
            glm::vec3 edge1 = p1 - p0;
            glm::vec3 edge2 = p2 - p0;

            glm::vec3 T = glm::normalize(modelVector * in.face.tangent);
            glm::vec3 B = glm::normalize(modelVector * in.face.bitangent);
            glm::vec3 N = glm::normalize(barycentricNormal);

            tangentSpace[0] = T;
            tangentSpace[1] = B;
            tangentSpace[2] = N;

            u32 x = textureWidth * uv.x;
            u32 y = textureHeight * uv.y;

            u32 offset = (y * textureWidth + x) * 4;

            glm::vec3 textureNormal = sampleNormalTexture(offset, in.normalMapTexture);
            glm::vec3 normal = glm::normalize(tangentSpace * textureNormal);

            float intensity = glm::dot(normal, glm::normalize(in.lightDir));
            if (intensity < 0) {
                intensity = 0;
            }

            glm::vec3 viewPos = in.camPos;
            glm::vec3 viewDir = glm::normalize(viewPos - frag);
            glm::vec3 halfwayDir = glm::normalize(in.lightDir + viewDir);

            glm::vec3 diffuseColor = sampleTexture(offset, in.diffuseTexture);
            glm::vec3 glowColor = sampleTexture(offset, in.glowTexture);
            float specWeight = sampleTexture(offset, in.specTexture)[0] / 255.0f;

            float shininess = 40.0f;
            float spec = pow(fmaxf(glm::dot(normal, halfwayDir), 0.0), shininess);

            glm::vec3 lightColor = glm::vec3(255, 200, 200);

            // shadow

            glm::vec3 originalPoint = glm::unProject(frag, modelView, in.projection, in.viewport);
            glm::vec3 lightSpacePoint =
                glm::project(glm::vec3(originalPoint), lightModelView, in.projection, in.viewport);
            lightSpacePoint.x = int(lightSpacePoint.x);
            lightSpacePoint.y = int(lightSpacePoint.y);

            int idx = lightSpacePoint.x + in.image.width * lightSpacePoint.y;
            float shadow = 0.3 + .7 * (in.image.shadowbuffer[idx] - 0.000005 < lightSpacePoint.z); //

            glm::vec3 color = glowColor * glm::vec3(2) +
                              (diffuseColor + lightColor * spec * specWeight) * intensity * shadow;
            //
            u8 rsrgb = u8(linearToSrgb(fminf(color.r, 255) / 255.0f) * 255);
            u8 gsrgb = u8(linearToSrgb(fminf(color.g, 255) / 255.0f) * 255);
            u8 bsrgb = u8(linearToSrgb(fminf(color.b, 255) / 255.0f) * 255);

            u32 color_out = rgbToU32(rsrgb, gsrgb, bsrgb);

            int coord = int(frag.x + frag.y * in.image.width);
            if (in.image.zbuffer[coord] < frag.z) {
                in.image.zbuffer[coord] = frag.z;
                drawPixel(frag.x, frag.y, color_out, in.image);
            }
        }
    }
}

void renderShape(Shape *shape, glm::mat4 projection, glm::mat4 view, glm::vec4 viewport, App *app) {

    glm::mat4 rotation = glm::rotate(glm::radians(app->rotateY), glm::vec3(0, 1, 0));
    glm::mat4 scale = glm::scale(glm::vec3(app->scale, app->scale, app->scale));
    glm::mat4 translation = glm::translate(glm::vec3(app->translateX, app->translateY, 0));

    glm::mat4 parentWorldMatrix = getParentMatrix((Node *)shape);
    glm::mat4 model = parentWorldMatrix * rotation;
    /* app->lightDir.y = cos(2 * glfwGetTime()); */
    /* app->lightDir.x = 2 *sin(2 * glfwGetTime()); */

    for (int i = 0; i < shape->faces.size(); i++) {

        DefaultVertexShaderIn vertexIn = {.face = shape->faces[i],
                                        .model = model,
                                        .view = view,
                                        .projection = projection,
                                        .viewport = viewport};

        DefaultVertexShaderOut vertexOut = runDefaultVertexProgram(vertexIn);

        UberFragmentShaderIn in = {.position = vertexOut.position,
                                    .face = shape->faces[i],
                                    .buffer = app->image.buffer,
                                    .bufferWidth = app->image.width,
                                    .bufferHeight = app->image.height,

                                    .lightDir = app->lightDir,
                                    .diffuseTexture = app->diffuseTexture,
                                    .normalMapTexture = app->normalMapTexture,
                                    .specTexture = app->specTexture,
                                    .glowTexture = app->glowTexture,

                                    .camPos = app->camera.pos,
                                    .model = model,
                                    .view = view,
                                    .projection = projection,
                                    .viewport = viewport,
                                    .image = app->image};

        runUberFragmentProgram(in);
    }
}

void renderShapeShadow(Shape *shape, glm::mat4 projection, glm::mat4 view, glm::vec4 viewport, App *app) {

    glm::mat4 rotation = glm::rotate(glm::radians(app->rotateY), glm::vec3(0, 1, 0));
    glm::mat4 scale = glm::scale(glm::vec3(app->scale, app->scale, app->scale));
    glm::mat4 translation = glm::translate(glm::vec3(app->translateX, app->translateY, 0));

    glm::mat4 parentWorldMatrix = getParentMatrix((Node *)shape);
    glm::mat4 model = parentWorldMatrix * rotation;
    /* app->lightDir.y = cos(2 * glfwGetTime()); */
    /* app->lightDir.x = 2 *sin(2 * glfwGetTime()); */

    for (int i = 0; i < shape->faces.size(); i++) {

        DefaultVertexShaderIn vertexIn = {.face = shape->faces[i],
                                        .model = model,
                                        .view = view,
                                        .projection = projection,
                                        .viewport = viewport};

        DefaultVertexShaderOut vertexOut = runDefaultVertexProgram(vertexIn);

        UberFragmentShaderIn in = {.position = vertexOut.position,
                                    .face = shape->faces[i],
                                    .buffer = app->image.buffer,
                                    .bufferWidth = app->image.width,
                                    .bufferHeight = app->image.height,

                                    .lightDir = app->lightDir,
                                    .diffuseTexture = app->diffuseTexture,
                                    .normalMapTexture = app->normalMapTexture,
                                    .specTexture = app->specTexture,
                                    .glowTexture = app->glowTexture,

                                    .camPos = app->camera.pos,
                                    .model = model,
                                    .view = view,
                                    .projection = projection,
                                    .viewport = viewport,
                                    .image = app->image};

        runShadowFragmentProgram(in);
    }
}

void renderWorld_r(Node *root, App *app, glm::mat4 projection, glm::mat4 view, glm::vec4 viewport) {
    std::vector<Node *> children = root->children;
    for (int i = 0; i < children.size(); i++) {
        Node *child = children[i];
        if (!strcmp(child->type, "shape")) {
            renderShape((Shape *)child, projection, view, viewport, app);
        }
        renderWorld_r(child, app, projection, view, viewport);
    }
}

void renderShadowMap_r(Node *root, App *app, glm::mat4 projection, glm::mat4 view, glm::vec4 viewport) {
    std::vector<Node *> children = root->children;
    for (int i = 0; i < children.size(); i++) {
        Node *child = children[i];
        if (!strcmp(child->type, "shape")) {
            renderShapeShadow((Shape *)child, projection, view, viewport, app);
        }
        renderShadowMap_r(child, app, projection, view, viewport);
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

    float zNear = 0.01f;
    float zFar = 1000.0f;

    glm::mat4 view = glm::lookAt(cam.pos, cam.target, cam.up);
    glm::mat4 projection = glm::perspectiveLH(cam.fov, width / float(height), 0.01f, 1000.0f);
    glm::vec4 viewport(0.0f, 0.0f, width - 1, height - 1);

    if (app->showAxis) {
        drawAxis(view, projection, viewport, image);
    }

    Node *root = app->world->worldRoot;

    renderShadowMap_r(root, app, projection, getLightView(app->lightDir), viewport);
    renderWorld_r(root, app, projection, view, viewport);
}

#endif // __TYPES_H__
