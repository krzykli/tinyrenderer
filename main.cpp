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

#define CR_HOST CR_UNSAFE
#include "cr.h"

#include "app.h"
#include "debug.h"
#include "obj-model.h"
#include "opengl-helpers.h"
#include "types.h"

App app;

int BUFFER_WIDTH = 1280;
int BUFFER_HEIGHT = 920;

u32 RED = 255 | (0 << 8) | (0 << 16) | (0 << 24);
u32 WHITE = 255 | (255 << 8) | (255 << 16) | (255 << 24);

const char *plugin = CR_PLUGIN("imalive");

void clearImage(Image &image) {
    for (int i = 0; i < image.width * image.height; i++) {
        image.buffer[i] = 0;
    }
}

struct HostData {
    int w, h;
    int display_w, display_h;
    ImGuiContext *imgui_context = nullptr;
    void *wndh = nullptr;

    // GLFW input/time data feed to guest
    double timestep = 0.0;
    float mouseWheel = 0.0f;
};

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {

        switch (key) {

        case GLFW_KEY_SPACE:
            app.displayUI = !app.displayUI;
            break;

        case GLFW_KEY_Q:
            app.isRunning = false;
            break;

        case GLFW_KEY_O:
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".obj",
                                                    "../obj/");
            break;
        }
    } else {
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

    app.translateX = 400;
    app.translateY = 400;
    app.scale = 150;

    app.turntable = true;
    app.turntableSpeed = 2;

    app.appTitle = "Tiny Renderer";
    app.renderMode = TRIANGLES;
}

void initImGui(GLFWwindow *window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void destroyImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

u32 float4ToU32(float *input) {
    int red = input[0] * 255;
    int green = input[1] * 255;
    int blue = input[2] * 255;
    int alpha = input[3] * 255;
    u32 output = red | (green << 8) | (blue << 16) | (alpha << 24);
    return output;
}

int main(int argc, char **argv) {
    initAppDefaults();
    GLFWwindow *window = initGLWindow(app.resolutionX, app.resolutionY, app.appTitle);
    if (window == NULL)
        return -1;

    glfwSetKeyCallback(window, keyCallback);
    initImGui(window);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    GLuint renderShaderProgramId = createShader("../shaders/render.vert", "../shaders/render.frag");

    app.image.buffer = (u32 *)malloc(BUFFER_WIDTH * BUFFER_HEIGHT * sizeof(u32));
    app.image.width = BUFFER_WIDTH;
    app.image.height = BUFFER_HEIGHT;

    GLuint renderTextureId;
    glGenTextures(1, &renderTextureId);

    GLuint renderVAO =
        initTextureRender(app.image.width, app.image.height, renderTextureId, renderShaderProgramId);

    double currentFrame = glfwGetTime();
    double lastFrame = currentFrame;
    bool lockFramerate = true;
    char fpsDisplay[12];

    std::string currentFile("../obj/armadillo.obj");
    loadOBJ(currentFile.c_str(), app.modelData.faces, app.modelData.vertices, app.modelData.uvs, app.modelData.normals);

    cr_plugin ctx;
    ctx.userdata = &app;
    cr_plugin_open(ctx, plugin);

    while (!glfwWindowShouldClose(window)) {

        currentFrame = glfwGetTime();
        app.deltaTime = currentFrame - lastFrame;

        double timeInMs = app.deltaTime * 1000.0f;
        double FPS;

        if (lockFramerate) {
            while (timeInMs < 16.666666f) {
                currentFrame = glfwGetTime();
                app.deltaTime = currentFrame - lastFrame;
                timeInMs = app.deltaTime * 1000.0f;
            }
        }
        lastFrame = currentFrame;
        if (app.deltaTime > 0.0001) {
            FPS = 1 / app.deltaTime;
        } else {
            FPS = 9999;
        }

        cr_plugin_update(ctx); // render

        glClear(GL_COLOR_BUFFER_BIT);

        glBindTexture(GL_TEXTURE_2D, renderTextureId);
        glActiveTexture(GL_TEXTURE0);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, app.image.width, app.image.height, GL_RGBA,
                        GL_UNSIGNED_BYTE, app.image.buffer);

        glBindVertexArray(renderVAO);
        glUseProgram(renderShaderProgramId);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        clearImage(app.image);

        if (app.displayUI) {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            //

            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Import Obj...")) {
                        ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File",
                                                                ".obj", "../obj/");
                    }
                    ImGui::EndMenu();
                }

                ImGui::EndMainMenuBar();
            }

            if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
                if (ImGuiFileDialog::Instance()->IsOk()) {
                    currentFile = ImGuiFileDialog::Instance()->GetFilePathName();
                    app.modelData.faces.clear();
                    app.modelData.vertices.clear();
                    app.modelData.uvs.clear();
                    app.modelData.normals.clear();

                    loadOBJ(currentFile.c_str(), app.modelData.faces, app.modelData.vertices,
                            app.modelData.uvs, app.modelData.normals);
                }
                ImGuiFileDialog::Instance()->Close();
            }

            ImGui::Begin("TinyRenderer");
            const char *items[] = {"Triangles", "Points", "Normals"};
            static const char *current_item = "Triangles";

            if (ImGui::BeginCombo("Render Mode", current_item)) {
                for (int n = 0; n < IM_ARRAYSIZE(items); n++) {
                    bool is_selected = (current_item == items[n]);
                    if (ImGui::Selectable(items[n], is_selected))
                        current_item = items[n];
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            if (current_item == "Triangles") {
                app.renderMode = TRIANGLES;
            } else if (current_item == "Points") {
                app.renderMode = POINTS;
            } else if (current_item == "Normals") {
                app.renderMode = NORMALS;
            }
            if (ImGui::CollapsingHeader("Settings")) {

                ImGui::Separator();
                ImGui::SliderFloat("translateX", &app.translateX, 0, BUFFER_WIDTH, "%.4f");
                ImGui::SliderFloat("translateY", &app.translateY, 0, BUFFER_HEIGHT, "%.4f");
                ImGui::SliderFloat("rotateY", &app.rotateY, -360, 360);
                ImGui::SliderFloat("scale", &app.scale, 1, 300.0);

                ImGui::Separator();
                ImGui::Checkbox("Turntable", &app.turntable);
                ImGui::SliderFloat("Speed", &app.turntableSpeed, 1, 5);
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
                ImGui::TextWrapped("Triangles : %lu", app.modelData.faces.size());
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

    cr_plugin_close(ctx);

    // Cleanup
    free(app.image.buffer);
    destroyImGui();

    glfwDestroyWindow(window);
    glfwTerminate();
}
