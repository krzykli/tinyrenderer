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

int BUFFER_WIDTH = 100;
int BUFFER_HEIGHT = 100;

#define persist static

//
u32 RED = 255 | (0 << 8) | (0 << 16) | (0 << 24);
u32 WHITE = 255 | (255 << 8) | (255 << 16) | (255 << 24);
//
typedef struct App {
    int resolutionX;
    int resolutionY;
    bool displayUI;
    bool isRunning;
    const char* appTitle;
} App;

App app;

persist void drawLadder(int x, int y, int height, int width, Image &image, u32 color) {
    drawLine(x, y, x, y + height, image, color);
    for (int i=1; i < height / 2 ; i++) {
        drawLine(x + 1, i * 2, x + width - 1, i * 2 , image, color);
    }
    drawLine(x + width, y, x + width, y + height, image, color);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS) {

        switch (key) {
            case GLFW_KEY_SPACE:
                app.displayUI = !app.displayUI;
                break;

            case GLFW_KEY_Q:
                app.isRunning = false;
        }
    }
}

void initAppDefaults() {
    app.resolutionX = BUFFER_WIDTH * 8;
    app.resolutionY = BUFFER_HEIGHT * 8;
    app.isRunning = true;
    app.displayUI = true;
    app.appTitle = "Tiny Renderer";
}

void initImGui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void destroyImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}


u32 float4ToU32(float* input) {
    int red = input[0] * 255;
    int green = input[1] * 255;
    int blue = input[2] * 255;
    int alpha = input[3] * 255;
    u32 output = red | (green << 8) | (blue << 16) | (alpha << 24);
    return output;
}

int main(int argc, char** argv) {
    initAppDefaults();
    GLFWwindow* window = initGLWindow(app.resolutionX, app.resolutionY, app.appTitle);
    if (window == NULL)
        return -1;

    glfwSetKeyCallback(window, keyCallback);
    initImGui(window);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    GLuint renderShaderProgramId = createShader("../shaders/render.vert", "../shaders/render.frag");

    Image image;
    image.buffer = (u32*)malloc(BUFFER_WIDTH * BUFFER_HEIGHT * sizeof(u32));
    image.width = BUFFER_WIDTH;
    image.height = BUFFER_HEIGHT;

    GLuint renderTextureId;
    glGenTextures(1, &renderTextureId);

    GLuint renderVAO = initTextureRender(image.width, image.height, renderTextureId, renderShaderProgramId);

    double currentFrame = glfwGetTime();
    double lastFrame = currentFrame;
    double deltaTime;
    bool lockFramerate = true;
    char fpsDisplay[12];

    while (!glfwWindowShouldClose(window)) {

        currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;

        double timeInMs = deltaTime * 1000.0f;
        double FPS;

        if (lockFramerate) {
            while (timeInMs < 16.666666f)
            {
                currentFrame = glfwGetTime();
                deltaTime = currentFrame - lastFrame;
                timeInMs = deltaTime * 1000.0f;
            }
        }
        lastFrame = currentFrame;
        if (deltaTime > 0.0001) {
            FPS = 1 / deltaTime;
        }
        else {
            FPS = 9999;
        }

        clearImage(image);
        glClear(GL_COLOR_BUFFER_BIT);

        //
        persist float lineColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};


        u32 lineColorU32 = float4ToU32(lineColor);
        drawLine(30, 50, 99, 60, image, lineColorU32);

        // render
        u8 red = int((sin(glfwGetTime() * 5 / 10.f) + 1) / 2 * 255);
        u8 green = int((sin(glfwGetTime() * 13 / 10.f) + 1) / 2 * 255);
        u8 blue = int((sin(glfwGetTime() * 27 / 10.f) + 1) / 2 * 255);
        u32 variableColor = red | (green << 8) | (blue << 16) | (255 << 24);

        //
        drawLadder(0, 0, 50, 5, image, variableColor);
        drawLadder(40, 0, 40, 7, image, variableColor);

        // Draw texture
        glBindTexture(GL_TEXTURE_2D, renderTextureId);
        glActiveTexture(GL_TEXTURE0);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width, image.height, GL_RGBA, GL_UNSIGNED_BYTE, image.buffer);

        glBindVertexArray(renderVAO);
        glUseProgram(renderShaderProgramId);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        if (app.displayUI) {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            //
            ImGui::Begin("Settings");
            ImGui::Checkbox("Lock Framerate", &lockFramerate);
            sprintf(fpsDisplay, "%lf", FPS);
            ImGui::LabelText("FPS", "%s", fpsDisplay);
            ImGui::End();
            //
            ImGui::Begin("Line Color", NULL, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::ColorEdit3("color", lineColor);
            ImGui::End();
            //
            ImGui::EndFrame();
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        if (!app.isRunning) {
            print("Terminating the application.");
            break;
        }
    }

    // Cleanup
    free(image.buffer);
    destroyImGui();

    glfwDestroyWindow(window);
    glfwTerminate();
}
