#include <assert.h>
#include <cmath>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/gtx/transform.hpp>

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

u32 RED = 255 | (0 << 8) | (0 << 16) | (0 << 24);
u32 WHITE = 255 | (255 << 8) | (255 << 16) | (255 << 24);

typedef struct App {
    int resolutionX;
    int resolutionY;
    bool displayUI;
    bool isRunning;
    float translateX;
    float translateY;
    float scale;
    const char* appTitle;
} App;

App app;

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
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".obj", "../obj/");
                break;
        }
    }
    else {
        switch (key) {

            case GLFW_KEY_H:
                app.translateX -= 50;
                break;

            case GLFW_KEY_L:
                app.translateX += 50;
                break;

            case GLFW_KEY_J:
                app.translateY -= 50;
                break;

            case GLFW_KEY_K:
                app.translateY += 50;
                break;

            case GLFW_KEY_UP:
                app.scale *= 1.1;
                break;

            case GLFW_KEY_DOWN:
                app.scale /= 1.1;
                break;
        }
    }
}

void initAppDefaults() {
    app.resolutionX = BUFFER_WIDTH;
    app.resolutionY = BUFFER_HEIGHT;
    app.isRunning = true;
    app.displayUI = true;
    app.translateX = 200;
    app.translateY = 200;
    app.scale = 100;
    app.appTitle = "Tiny Renderer";
}

void initImGui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

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
    bool turntable = true;
    float turntableSpeed = 2;
    char fpsDisplay[12];

    std::string currentFile ("../obj/armadillo.obj");
    std::vector<Face> faces;
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

        persist float rotateY = 180.0f;
        if (turntable) {
            rotateY = rotateY + turntableSpeed * deltaTime * 20;
            if (rotateY > 360.f) {
                rotateY = 0;
            }
        }

        for (int i = 0; i < faces.size(); i++) {
            Face f = faces[i];
            Vec2i screenCoords[3];
            glm::mat4 rotation = glm::rotate(glm::radians(rotateY), glm::vec3(0, 1, 0));

            for (int j = 0; j < 3; j++) {
                Vec3f v0 = f.verts[j];
                glm::vec4 vertex = glm::vec4(v0.x, v0.y, v0.z, 1);
                glm::vec4 rotatedVertex = rotation * vertex;
                int x = rotatedVertex.x * app.scale + app.translateX;
                int y = rotatedVertex.y * app.scale + app.translateY;
                screenCoords[j] = { .x = x, .y = y};
            }

            Vec3f normalA = f.normals[0];
            Vec3f normalB = f.normals[1];
            Vec3f normalC = f.normals[2];

            Vec3f v0 = f.verts[0];
            Vec3f v1 = f.verts[1];
            Vec3f v2 = f.verts[2];

            float avNormalX = (normalA.x + normalB.x + normalC.x) / 3;
            float avNormalY = (normalA.y + normalB.y + normalC.y) / 3;
            float avNormalZ = (normalA.z + normalB.z + normalC.z) / 3;

            u32 color = int((avNormalX + 1) / 2 * 255) | int((avNormalY + 1) / 2 * 255)  << 8 | int((avNormalZ + 1) / 2 * 255) << 16 | (0 << 24);
            drawTriangle(screenCoords[0], screenCoords[1], screenCoords[2], image, color);
        }

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
                        ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".obj", "../obj/");
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
                ImGui::SliderFloat("translateX", &app.translateX, 0, BUFFER_WIDTH, "%.4f");
                ImGui::SliderFloat("translateY", &app.translateY, 0, BUFFER_HEIGHT, "%.4f");
                ImGui::SliderFloat("rotateY", &rotateY, -360, 360);
                ImGui::SliderFloat("scale", &app.scale, 1, 300.0);
                ImGui::Separator();
                ImGui::Checkbox("Turntable", &turntable);
                ImGui::SliderFloat("Speed", &turntableSpeed, 1, 5);
                ImGui::Separator();
                ImGui::Checkbox("Lock Framerate", &lockFramerate);
            }

            if (ImGui::CollapsingHeader("Info")) {
                sprintf(fpsDisplay, "%lf", FPS);
                ImGui::Text("FPS");
                ImGui::SameLine();
                ImGui::Text("%s", fpsDisplay);
                ImGui::Separator();
                ImGui::TextWrapped("Model: %s", currentFile.c_str());
                ImGui::TextWrapped("Triangles : %lu", faces.size());
                ImGui::Separator();
            }

            ImGui::Spacing();
            ImGui::Text("Keyboard shortcuts:");
            ImGui::Text("'hjkl': translate");
            ImGui::Text("'o': open a new file");
            ImGui::Text("'q': exit");

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
