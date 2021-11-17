#ifndef __APP_H__
#define __APP_H__

#include "types.h"

typedef struct App {
    int resolutionX;
    int resolutionY;
    Image image;

    bool displayUI;
    bool isRunning;
    bool turntable;
    float turntableSpeed;
    RenderMode renderMode;

    ModelData modelData;

    float translateX;
    float translateY;
    float rotateY;
    float scale;

    double deltaTime;

    const char *appTitle;
} App;

#endif // __APP_H__
