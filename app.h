#ifndef __APP_H__
#define __APP_H__

#include "types.h"

typedef struct App {
    int resolutionX;
    int resolutionY;
    Image image;
    Camera camera;

    float lastX;
    float lastY;

    float normalLength;
    bool displayUI;
    bool showAxis;
    bool isRunning;
    bool turntable;
    float turntableSpeed;
    glm::vec3 lightDir;
    RenderMode renderMode;

    Png diffuseTexture;
    Png normalMapTexture;

    float translateX;
    float translateY;
    float rotateY;
    float scale;

    double deltaTime;

    World* world;

    const char *appTitle;
} App;

#endif // __APP_H__
