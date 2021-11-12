#include <assert.h>
#include <cmath>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "thirdparty/imgui/imgui.h"
#include "thirdparty/imgui/imgui_impl_glfw.h"
#include "thirdparty/imgui/imgui_impl_opengl3.h"
#include "thirdparty/imguiFileDialog/ImGuiFileDialog.h"

#include "debug.h"
#include "image.h"
#include "obj-model.h"
#include "opengl-helpers.h"
#include "types.h"


int BUFFER_WIDTH = 800;
int BUFFER_HEIGHT = 600;

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
        drawLine(x + 1, y + i * 2, x + width - 1, y + i * 2 , image, color);
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
                break;

            case GLFW_KEY_O:
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".obj", ".");
                break;
        }
    }
}

void initAppDefaults() {
    app.resolutionX = BUFFER_WIDTH;
    app.resolutionY = BUFFER_HEIGHT;
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

    std::string currentFile ("../obj/african_head.obj");
    std::vector<std::vector<int>> faces;
    std::vector<Vec3f> vertices;
    std::vector<Vec3f> uvs;
    std::vector<Vec3f> normals;
    loadOBJ(currentFile.c_str(), faces, vertices, uvs, normals);

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

        persist float lineColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};

        persist float translation[2] = {2.0f, 2.0f};

        persist float scale = 100;

        u32 lineColorU32 = float4ToU32(lineColor);

        for (int i=0; i < faces.size(); i++) {
            std::vector<int> face = faces[i];
            for (int j=0; j<3; j++) {
                Vec3f v0 = vertices[face[j]];
                Vec3f v1 = vertices[face[(j+1)%3]];
                int x0 = (v0.x + translation[0]) * scale;
                int y0 = (v0.y + translation[1]) * scale;
                int x1 = (v1.x + translation[0]) * scale;
                int y1 = (v1.y + translation[1]) * scale;
                drawLine(x0, y0, x1, y1, image, lineColorU32);
            } 
        }

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

            if(ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if(ImGui::MenuItem("Import Obj..."))
                    {
                        ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".obj", ".");
                    }
                    ImGui::EndMenu();
                }

                ImGui::EndMainMenuBar();
            }

            if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) 
            {
                if (ImGuiFileDialog::Instance()->IsOk())
                {
                    currentFile = ImGuiFileDialog::Instance()->GetFilePathName();
                    faces.clear();
                    vertices.clear();
                    uvs.clear();
                    normals.clear();

                    loadOBJ(currentFile.c_str(), faces, vertices, uvs, normals);
                }
                ImGuiFileDialog::Instance()->Close();
            }

            ImGui::Begin("TinyRenderer");
            if (ImGui::CollapsingHeader("Settings")) {
                ImGui::SliderFloat2("translation", translation, -10, 10.0);
                ImGui::SliderFloat("scale", &scale, 1, 300.0);
                ImGui::ColorEdit3("wireframe", lineColor);
                ImGui::Checkbox("Lock Framerate", &lockFramerate);
            }

            if (ImGui::CollapsingHeader("Info")) {
                sprintf(fpsDisplay, "%lf", FPS);
                ImGui::Text("FPS");
                ImGui::SameLine();
                ImGui::Text("%s", fpsDisplay);
                ImGui::Separator();
                ImGui::TextWrapped("Model: %s", currentFile.c_str());
            }

            ImGui::End();


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
