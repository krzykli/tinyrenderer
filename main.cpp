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
int leftButtonDrag = false;
int middleButtonDrag = false;

float zdepthExponent = 0.003f;

int BUFFER_WIDTH = 1280;
int BUFFER_HEIGHT = 920;

const char *plugin = CR_PLUGIN("imalive");

inline void Style() {
    /// 0 = FLAT APPEARENCE
    /// 1 = MORE "3D" LOOK
    int is3D = 1;
    ImGui::GetIO().Fonts->AddFontFromFileTTF("../fonts/hackNerd.ttf", 13.0f);

    ImVec4 *colors = ImGui::GetStyle().Colors;

    ImVec4 darkBlue = ImVec4(0.1f, 0.3f, 0.7f, 0.54f);
    ImVec4 brightBlue = ImVec4(0.2f, 0.6f, 0.8f, 0.54f);

    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
    colors[ImGuiCol_Border] = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
    colors[ImGuiCol_CheckMark] = brightBlue;
    colors[ImGuiCol_SliderGrab] = brightBlue;
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.33f, 0.70f, 0.90f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 1.f);
    colors[ImGuiCol_HeaderHovered] = brightBlue;
    colors[ImGuiCol_HeaderActive] = darkBlue;
    colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(8.00f, 8.00f);
    style.FramePadding = ImVec2(5.00f, 2.00f);
    style.CellPadding = ImVec2(6.00f, 6.00f);
    style.ItemSpacing = ImVec2(6.00f, 6.00f);
    style.ItemInnerSpacing = ImVec2(6.00f, 6.00f);
    style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
    style.IndentSpacing = 25;
    style.ScrollbarSize = 15;
    style.GrabMinSize = 10;
    style.WindowBorderSize = 1;
    style.ChildBorderSize = 1;
    style.PopupBorderSize = 1;
    style.FrameBorderSize = 1;
    style.TabBorderSize = 1;
    style.WindowRounding = 7;
    style.ChildRounding = 4;
    style.FrameRounding = 3;
    style.PopupRounding = 4;
    style.ScrollbarRounding = 9;
    style.GrabRounding = 3;
    style.LogSliderDeadzone = 4;
    style.TabRounding = 4;
}

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

    return {.image = image, .width = width, .height = height};
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
            leftButtonDrag = true;
        else if (GLFW_RELEASE == action)
            leftButtonDrag = false;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        if (GLFW_PRESS == action)
            middleButtonDrag = true;
        else if (GLFW_RELEASE == action)
            middleButtonDrag = false;
    }
}

void scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    float sensitivity = 0.05f;

    yoffset = fmin(yoffset, 1.0f);
    yoffset = fmax(yoffset, -1.3f);
    glm::vec3 direction = app.camera.pos - app.camera.target;
    glm::vec3 offset = direction * (float)yoffset * sensitivity;
    glm::vec3 new_pos = app.camera.pos + offset;
    float distance = glm::distance(new_pos, app.camera.target);
    if (distance < 1.0f)
        offset = glm::vec3(0, 0, 0);

    app.camera.pos -= offset;
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

    float sensitivity = 0.01f;

    glm::mat4 view = glm::lookAt(app.camera.pos, app.camera.target, app.camera.up);

    if (leftButtonDrag) {
        GLFWcursor *cursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
        glfwSetCursor(window, cursor);

        float deltaTheta = -xoffset * sensitivity;
        float deltaPhi = -yoffset * sensitivity;
        if (deltaTheta) {
            glm::vec3 pivot = app.camera.target;
            glm::vec3 axis = glm::vec3(0, 1, 0);
            glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1), deltaTheta, axis);
            view = view * rotationMatrix;
        }
        if (deltaPhi) {
            // To rotate the camera without flipping we need to rotate
            // it by moving it to origin and resetting it back.
            glm::vec3 pivot = glm::vec3(view * glm::vec4(app.camera.target, 1.0f));
            glm::vec3 axis = glm::vec3(1, 0, 0);
            glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1), deltaPhi, axis);
            glm::mat4 rotationWithPivot = glm::translate(glm::mat4(1), pivot) * rotationMatrix *
                                          glm::translate(glm::mat4(1), -pivot);
            // Apply the rotation after the current view
            view = rotationWithPivot * view;
        }
    } else if (middleButtonDrag) {
        glm::vec3 panHorizontal;
        glm::vec3 panVertical = app.camera.up;
        glm::vec3 direction = glm::normalize(app.camera.pos - app.camera.target);
        glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), direction));
        if (panVertical.y > 0)
            panHorizontal = -right;
        else
            panHorizontal = right;

        float distance = glm::distance(app.camera.pos, app.camera.target);
        float sensitivity = distance * 0.001f;
        float deltaPanH = -xoffset * sensitivity;
        float deltaPanV = -yoffset * sensitivity;

        glm::vec3 offset = panHorizontal * deltaPanH + panVertical * deltaPanV;
        app.camera.pos += offset;
        app.camera.target += offset;

        view = glm::lookAt(app.camera.pos, app.camera.target, app.camera.up);
    }

    glm::mat4 camera_world = glm::inverse(view);
    float targetDist = glm::length(app.camera.target - app.camera.pos);
    app.camera.pos = glm::vec3(camera_world[3]);
    app.camera.target = app.camera.pos - glm::vec3(camera_world[2]) * targetDist;
    app.camera.up = glm::vec3(camera_world[1]);
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {

    const float cameraSpeed = 0.5f; // adjust accordingly

    if (action == GLFW_PRESS) {

        switch (key) {

        case GLFW_KEY_1:
            app.renderMode = TRIANGLES;
            break;

        case GLFW_KEY_2:
            app.renderMode = POINTS;
            break;

        case GLFW_KEY_3:
            app.renderMode = NORMALS;
            break;

        case GLFW_KEY_4:
            app.renderMode = ZBUFFER;
            break;

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

        case GLFW_KEY_T:
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".png",
                                                    "../textures/");
            break;
        }
    } else {
        switch (key) {

        case GLFW_KEY_X:
            app.showAxis = !app.showAxis;
            break;

        case GLFW_KEY_UP:
            app.camera.pos += cameraSpeed * app.camera.up;
            break;

        case GLFW_KEY_DOWN:
            app.camera.pos -= cameraSpeed * app.camera.up;
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
    cam.pos = glm::vec3(3, 3, 3);
    cam.target = glm::vec3(0, 0, 0);
    cam.up = glm::vec3(0, 1, 0);
    cam.fov = 45.f;
    cam.yaw =
        -90.0f; // yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction
                // vector pointing to the right so we initially rotate a bit to the left.
    cam.pitch = 0.0f;
    app.camera = cam;
    app.pngInfo = {};

    app.normalLength = 0.1f;
    app.lightDir = glm::vec3(1, 1, 1);

    app.showAxis = true;
    app.turntable = false;
    app.turntableSpeed = 2;

    app.appTitle = "Tiny Renderer";
    app.renderMode = TRIANGLES;
}

void initImGui(GLFWwindow *window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    Style();

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

const char *getCurrentItemFromCurrentRenderMode() {
    switch (app.renderMode) {
    case TRIANGLES:
        return "Triangles";
    case POINTS:
        return "Points";
    case NORMALS:
        return "Normals";
    case ZBUFFER:
        return "Zbuffer";
    }
}

int main(int argc, char **argv) {
    initAppDefaults();
    GLFWwindow *window = initGLWindow(app.resolutionX, app.resolutionY, app.appTitle);
    if (window == NULL)
        return -1;

    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, cursorCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scrollCallback);
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

    std::string currentObj("../obj/african_head.obj");
    loadOBJ(currentObj.c_str(), app.modelData.faces, app.modelData.vertices, app.modelData.uvs,
            app.modelData.normals);

    std::string currentTexture("../textures/african_head_diffuse.png");
    app.pngInfo = loadPNG(currentTexture.c_str());

    cr_plugin ctx;
    ctx.userdata = &app;
    cr_plugin_open(ctx, plugin);

    while (!glfwWindowShouldClose(window)) {

        currentFrame = glfwGetTime();
        app.deltaTime = currentFrame - lastFrame;

        double timeInMs = app.deltaTime * 1000.0f;
        double FPS;

        if (lockFramerate) {
            if (timeInMs < 16.6666666f) {
                while (timeInMs < 16.666666f) {
                    currentFrame = glfwGetTime();
                    app.deltaTime = currentFrame - lastFrame;
                    timeInMs = app.deltaTime * 1000.0f;
                }
            } else if (timeInMs > 16.6666666f && timeInMs < 33.333333f) {
                while (timeInMs < 33.333333f) {
                    currentFrame = glfwGetTime();
                    app.deltaTime = currentFrame - lastFrame;
                    timeInMs = app.deltaTime * 1000.0f;
                }
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
        if (app.renderMode == ZBUFFER) {
            for (int i = 0; i < app.image.width * app.image.height; i++) {
                float z = app.image.zbuffer[i];
                if (z > 0) {
                    app.image.zbuffer[i] = powf(z, zdepthExponent);
                }
            }
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
                    if (ImGui::MenuItem("Import Texture...")) {
                        ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File",
                                                                ".png", "../textures/");
                    }
                    ImGui::EndMenu();
                }

                ImGui::EndMainMenuBar();
            }

            if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {

                if (ImGuiFileDialog::Instance()->IsOk()) {
                    std::string path = ImGuiFileDialog::Instance()->GetFilePathName();
                    if (strstr(path.c_str(), ".obj")) {
                        currentObj = path;
                        app.modelData.faces.clear();
                        app.modelData.vertices.clear();
                        app.modelData.uvs.clear();
                        app.modelData.normals.clear();

                        loadOBJ(currentObj.c_str(), app.modelData.faces, app.modelData.vertices,
                                app.modelData.uvs, app.modelData.normals);
                    } else if (strstr(path.c_str(), ".png")) {
                        currentTexture = path;
                        free(app.pngInfo.image);
                        app.pngInfo = loadPNG(path.c_str());
                    }
                }
                ImGuiFileDialog::Instance()->Close();
            }

            ImGui::Begin("TinyRenderer");
            const char *items[] = {"Triangles", "Points", "Normals", "Zbuffer"};
            const char *current_item = getCurrentItemFromCurrentRenderMode();

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
            } else if (current_item == "Zbuffer") {
                app.renderMode = ZBUFFER;
            }

            if (ImGui::CollapsingHeader("Settings")) {
                ImGui::Separator();

                ImGui::SliderFloat3("light dir", &app.lightDir.x, -5.0f, 5.0f);
                ImGui::SliderFloat("normal length", &app.normalLength, 0.01f, 1.0f);
                ImGui::SliderFloat("zdepth exp", &zdepthExponent, 0.001f, 0.015f);

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
                ImGui::TextWrapped("Model: %s", currentObj.c_str());
                ImGui::TextWrapped("Texture: %s", currentTexture.c_str());
                ImGui::TextWrapped("Triangles : %lu", app.modelData.faces.size());
                ImGui::Separator();
            }

            ImGui::Spacing();
            ImGui::Text("Keyboard shortcuts:");
            ImGui::Text("'left mouse': orbit");
            ImGui::Text("'middle mouse': pan");
            ImGui::Text("'scroll mouse': zoom");
            ImGui::Text("'1-4': display modes");
            ImGui::Text("'o': open a new model");
            ImGui::Text("'t': open a new texture");
            ImGui::Text("'z': show z-buffer");
            ImGui::Text("'x': show axis");
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
    free(app.pngInfo.image);
    free(app.image.buffer);
    free(app.image.zbuffer);
    destroyImGui();

    glfwDestroyWindow(window);
    glfwTerminate();
}
