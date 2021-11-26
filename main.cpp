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
#include "thirdparty/lodepng/lodepng.h"
#include "types.h"

App app;
bool firstMouse = false;
int mouseButtonDrag = false;

bool showZbuffer = false;

int BUFFER_WIDTH = 1280;
int BUFFER_HEIGHT = 920;

const char *plugin = CR_PLUGIN("imalive");

void clearImage(Image &image) {
    for (int i = 0; i < image.width * image.height; i++) {
        image.buffer[i] = 0;
        image.zbuffer[i] = 0;
    }
}

Png loadPNG(const char *filename) {
    unsigned error;
    unsigned char *image = 0;
    unsigned width, height;

    error = lodepng_decode32_file(&image, &width, &height, filename);
    if (error)
        printf("error %u: %s\n", error, lodepng_error_text(error));

    return {.image = image, .width = width, .height = height };
}

void flipImageVertically(Image &image) {
    int width = image.width;
    int height = image.height;
    unsigned rows = height / 2;
    unsigned *tempRow = (u32 *)malloc(width * sizeof(u32));

    for (unsigned rowIndex = 0; rowIndex < rows; rowIndex++) {
        memcpy(tempRow, image.buffer + rowIndex * width, width * sizeof(u32));
        memcpy(image.buffer + rowIndex * width, image.buffer + (height - rowIndex - 1) * width,
               width * sizeof(u32));
        memcpy(image.buffer + (height - rowIndex - 1) * width, tempRow, width * sizeof(u32));
    }

    free(tempRow);

    float *tempRowZdepth = (float *)malloc(width * sizeof(float));

    for (int rowIndex = 0; rowIndex < rows; rowIndex++) {
        memcpy(tempRowZdepth, image.zbuffer + rowIndex * width, width * sizeof(float));
        memcpy(image.zbuffer + rowIndex * width, image.zbuffer + (height - rowIndex - 1) * width,
               width * sizeof(float));
        memcpy(image.zbuffer + (height - rowIndex - 1) * width, tempRowZdepth,
               width * sizeof(float));
    }

    free(tempRowZdepth);
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

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (GLFW_PRESS == action)
            mouseButtonDrag = true;
        else if (GLFW_RELEASE == action)
            mouseButtonDrag = false;
    }
}

void cursorCallback(GLFWwindow *window, double xpos, double ypos) {
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return;
    }

    if (firstMouse) {
        app.lastX = xpos;
        app.lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - app.lastX;
    float yoffset = app.lastY - ypos; // reversed since y-coordinates go from bottom to top

    app.lastX = xpos;
    app.lastY = ypos;

    float sensitivity = 0.1f;

    if (mouseButtonDrag) {

        xoffset *= sensitivity;
        yoffset *= sensitivity;

        app.camera.yaw -= xoffset;
        app.camera.pitch += yoffset;

        if (app.camera.pitch > 89.0f)
            app.camera.pitch = 89.0f;
        if (app.camera.pitch < -89.0f)
            app.camera.pitch = -89.0f;

        glm::vec3 front;
        front.x = cos(glm::radians(app.camera.yaw)) * cos(glm::radians(app.camera.pitch));
        front.y = sin(glm::radians(app.camera.pitch));
        front.z = sin(glm::radians(app.camera.yaw)) * cos(glm::radians(app.camera.pitch));
        app.camera.front = glm::normalize(front);
    }
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {

    const float cameraSpeed = 0.5f; // adjust accordingly

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

        case GLFW_KEY_Z:
            showZbuffer = !showZbuffer;
            break;

        case GLFW_KEY_X:
            app.showAxis = !app.showAxis;
            break;

        case GLFW_KEY_UP:
            app.camera.pos += cameraSpeed * app.camera.up;
            break;

        case GLFW_KEY_DOWN:
            app.camera.pos -= cameraSpeed * app.camera.up;
            break;

        case GLFW_KEY_W:
            app.camera.pos -= cameraSpeed * app.camera.front;
            break;

        case GLFW_KEY_S:
            app.camera.pos += cameraSpeed * app.camera.front;
            break;

        case GLFW_KEY_A:
            app.camera.pos +=
                glm::normalize(glm::cross(app.camera.front, app.camera.up)) * cameraSpeed;
            break;

        case GLFW_KEY_D:
            app.camera.pos -=
                glm::normalize(glm::cross(app.camera.front, app.camera.up)) * cameraSpeed;
            break;
        }
    }
}

void initAppDefaults() {
    app.resolutionX = BUFFER_WIDTH;
    app.resolutionY = BUFFER_HEIGHT;
    app.isRunning = true;
    app.displayUI = true;

    // controls
    app.lastX = BUFFER_WIDTH / 2.0;
    app.lastY = BUFFER_HEIGHT / 2.0;

    Camera cam;
    cam.pos = glm::vec3(0, 0, -10);
    cam.front = glm::vec3(0, 0, -1);
    cam.up = glm::vec3(0, 1, 0);
    cam.fov = 45.f;
    cam.yaw =
        -90.0f; // yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction
                // vector pointing to the right so we initially rotate a bit to the left.
    cam.pitch = 0.0f;
    app.camera = cam;

    app.normalLength = 0.1f;

    app.showAxis = true;
    app.turntable = true;
    app.turntableSpeed = 2;

    app.appTitle = "Tiny Renderer";
    app.renderMode = TRIANGLES;
}

void initImGui(GLFWwindow *window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
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
    glfwSetCursorPosCallback(window, cursorCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    initImGui(window);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    GLuint renderShaderProgramId = createShader("../shaders/render.vert", "../shaders/render.frag");

    app.image.buffer = (u32 *)malloc(BUFFER_WIDTH * BUFFER_HEIGHT * sizeof(u32));
    app.image.zbuffer = (float *)malloc(BUFFER_WIDTH * BUFFER_HEIGHT * sizeof(float));
    app.image.width = BUFFER_WIDTH;
    app.image.height = BUFFER_HEIGHT;

    GLuint renderTextureId;
    glGenTextures(1, &renderTextureId);

    GLuint renderVAO = initTextureRender(app.image.width, app.image.height, renderTextureId,
                                         renderShaderProgramId);

    double currentFrame = glfwGetTime();
    double lastFrame = currentFrame;
    bool lockFramerate = true;
    char fpsDisplay[12];

    std::string currentFile("../obj/african_head.obj");
    loadOBJ(currentFile.c_str(), app.modelData.faces, app.modelData.vertices, app.modelData.uvs,
            app.modelData.normals);

    app.pngInfo = loadPNG("../african_head_diffuse.png");

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
        flipImageVertically(app.image);

        glClear(GL_COLOR_BUFFER_BIT);

        glBindTexture(GL_TEXTURE_2D, renderTextureId);
        glActiveTexture(GL_TEXTURE0);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        if (showZbuffer) {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, app.image.width, app.image.height, GL_RGBA,
                            GL_UNSIGNED_BYTE, app.image.zbuffer);
        } else {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, app.image.width, app.image.height, GL_RGBA,
                            GL_UNSIGNED_BYTE, app.image.buffer);
        }

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

                ImGui::SliderFloat("normalLength", &app.normalLength, 0.01f, 1.0f);

                ImGui::Separator();
                ImGui::Checkbox("Turntable", &app.turntable);
                ImGui::SliderFloat("rotateY", &app.rotateY, -360, 360);
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
            ImGui::Text("'wasd up/down': move camera");
            ImGui::Text("'o': open a new file");
            ImGui::Text("'z': show z-buffer");
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
    free(app.image.zbuffer);
    destroyImGui();

    glfwDestroyWindow(window);
    glfwTerminate();
}
