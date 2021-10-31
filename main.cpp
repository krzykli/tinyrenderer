#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "render.h"
#include "opengl-helpers.h"

#define print(format, ...) \
    printf("%s\t| %s:%d\t| " format "\n", __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)

int WINDOW_WIDTH = 100;
int WINDOW_HEIGHT = 100;
const char* WINDOW_TITLE = "Tiny Renderer";

void line(int x0, int y0, int x1, int y1, Image &image, uint32_t color) { 
    bool steep = false; 
    if (std::abs(x0-x1)<std::abs(y0-y1)) { // if the line is steep, we transpose the image 
        std::swap(x0, y0); 
        std::swap(x1, y1); 
        steep = true; 
    } 
    if (x0>x1) { // make it left−to−right 
        std::swap(x0, x1); 
        std::swap(y0, y1); 
    } 
    for (int x=x0; x<=x1; x++) { 
        float t = (x-x0)/(float)(x1-x0); 
        int y = y0*(1.-t) + y1*t; 
        if (steep) { 
            drawPixel(y, x, color, image); // if transposed, de−transpose 
        } else { 
            drawPixel(x, y, color, image); 
        } 
    } 
}

int main(int argc, char** argv) {
    GLFWwindow* window = initGLWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    if (window == NULL)
        return -1;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    GLuint renderShaderProgramId = createShader("../shaders/render.vert", "../shaders/render.frag");

    Image image;
    image.buffer = (uint32_t*)malloc(WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(uint32_t));
    image.width = WINDOW_WIDTH;
    image.height = WINDOW_HEIGHT;

    for (int i=0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++) {
        image.buffer[i] = 0;
    }

    //

    uint32_t red = 255 | (0 << 8) | (0 << 16) | (255 << 24);  // ABGR
    uint32_t white = 255 | (0 << 8) | (0 << 16) | (0 << 24);
    line(13, 20, 80, 40, image, white); 
    line(20, 13, 40, 80, image, red); 
    line(80, 40, 13, 20, image, red);

    // 

    GLuint renderTextureId;
    glGenTextures(1, &renderTextureId);

    GLuint renderVAO = initTextureRender(image.width, image.height, renderTextureId, renderShaderProgramId);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);
        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderTextureId);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width, image.height, GL_RGBA, GL_UNSIGNED_BYTE, image.buffer);

        glUseProgram(renderShaderProgramId);
        glBindVertexArray(renderVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glUseProgram(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glEnable(GL_DEPTH_TEST);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    free(image.buffer);
    glfwTerminate();
}
