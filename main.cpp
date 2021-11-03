#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "thirdparty/imgui/imgui.h"
#include "thirdparty/imgui/imgui_impl_glfw.h"
#include "thirdparty/imgui/imgui_impl_opengl3.h"

#include "types.h"
#include "image.h"
#include "opengl-helpers.h"
#include "debug.h"

int WINDOW_WIDTH = 100;
int WINDOW_HEIGHT = 100;

const char* WINDOW_TITLE = "Tiny Renderer";

//
u32 RED = 255 | (0 << 8) | (0 << 16) | (0 << 24);
u32 WHITE = 255 | (255 << 8) | (255 << 16) | (255 << 24);
//


void line(int x0, int y0, int x1, int y1, Image &image, u32 color) { 

    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }

    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int dx = x1 - x0;
    int dy = y1 - y0;

    int derror2 = std::abs(dy) * 2;
    int error2 = 0;
    int y = y0;

    for (int x = x0; x <= x1; x++) {
        if (steep) {
            drawPixel(y, x, color, image);
        } else {
            drawPixel(x, y, color, image);
        }

        error2 += derror2;
        if (error2 > .5) {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx * 2;
        }
    }
}

int main(int argc, char** argv) {
    GLFWwindow* window = initGLWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    if (window == NULL)
        return -1;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    GLuint renderShaderProgramId = createShader("../shaders/render.vert", "../shaders/render.frag");

    Image image;
    image.buffer = (u32*)malloc(WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(u32));
    image.width = WINDOW_WIDTH;
    image.height = WINDOW_HEIGHT;
    clearImage(image);

    GLuint renderTextureId;
    glGenTextures(1, &renderTextureId);

    GLuint renderVAO = initTextureRender(image.width, image.height, renderTextureId, renderShaderProgramId);


    double currentFrame = glfwGetTime();
    double lastFrame = currentFrame;
    double deltaTime;

    double lastY = 0;
    bool lockFramerate = true;

    while (!glfwWindowShouldClose(window)) {

        currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        float timeInMs = deltaTime * 1000.0f;

        if (lockFramerate) {
            while (timeInMs < 16.666f)
            {
                currentFrame = glfwGetTime();
                deltaTime = currentFrame - lastFrame;
                timeInMs = deltaTime * 1000.0f;
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);
        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderTextureId);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // strobe light
        float lastYOffset = (sin(lastY) + 1)/2;
        float midPoint = 50;
        line(30, 50, 99, 60, image, RED);
        line(0, 0, 30, 99, image, WHITE);

        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width, image.height, GL_RGBA, GL_UNSIGNED_BYTE, image.buffer);

        glUseProgram(renderShaderProgramId);
        glBindVertexArray(renderVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glUseProgram(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glEnable(GL_DEPTH_TEST);

        clearImage(image);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    free(image.buffer);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}
