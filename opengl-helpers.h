#ifndef OPENGLUTILS
#define OPENGLUTILS

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "types.h"
#include "debug.h"


void loadFileContents(const char* file_path, char* buffer)
{
    FILE* fh;
    fh = fopen(file_path, "r");
    fread(buffer, 1000, 1, fh);  // FIXME
    fclose(fh);
}


uint8_t compileShader(GLuint shader_id, const char* shader_path)
{
    print("Compiling shader %s", shader_path);

    u32 buffer_size = 1000;
    char* shaderBuffer = (char*)calloc(1, sizeof(char) * buffer_size);
    loadFileContents(shader_path, shaderBuffer);
    glShaderSource(shader_id, 1, &shaderBuffer, NULL);
    glCompileShader(shader_id);
    free(shaderBuffer);

    GLint is_compiled = 0;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &is_compiled);
    if(is_compiled == GL_FALSE)
    {
        GLint max_length = 0;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &max_length);
        char errorLog[max_length];
        glGetShaderInfoLog(shader_id, max_length, &max_length, &errorLog[0]);
        glDeleteShader(shader_id);
        print("%s", errorLog);
        return 0;
    }
    return 1;
}

GLuint createShader(const char* vertex_shader, const char* fragment_shader)
{
    GLuint shader_program_id = glCreateProgram();
    GLuint vert_id = glCreateShader(GL_VERTEX_SHADER);
    uint8_t rv = compileShader(vert_id, vertex_shader);
    assert(rv);

    glAttachShader(shader_program_id, vert_id);
    GLuint frag_id = glCreateShader(GL_FRAGMENT_SHADER);
    rv = compileShader(frag_id, fragment_shader);
    assert(rv);

    glAttachShader(shader_program_id, frag_id);
    glLinkProgram(shader_program_id);
    return shader_program_id;
}

GLFWwindow* initGLWindow(int width, int height, const char* title) {
    GLFWwindow* window;

    if (!glfwInit())
        return NULL;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(width, height, title, NULL, NULL);
    /* glfwMaximizeWindow(window); */

    if (!window) {
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit())
        return NULL;

    return window;
}

GLuint initTextureRender(int width, int height, GLuint renderTextureId, GLuint programId) {

    glBindTexture(GL_TEXTURE_2D, renderTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glUseProgram(programId);
    glUniform1i(glGetUniformLocation(programId, "texture1"), 0);
    glUseProgram(0);

    glBindTexture(GL_TEXTURE_2D, 0);

    // https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/4.2.textures_combined/textures_combined.cpp
    float renderVertices[] = {
        // positions          // colors           // texture coords
        -1.0f, 1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // top right
        -1.0f, -1.0f, 0.0f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f, // bottom right
        1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f, // bottom left
        1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   1.0f, 1.0f  // top left 
    };

    u32 renderIndices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    GLuint render_VAO, render_VBO, render_EBO;
    glGenVertexArrays(1, &render_VAO);
    glGenBuffers(1, &render_VBO);
    glGenBuffers(1, &render_EBO);

    glBindVertexArray(render_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, render_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(renderVertices), renderVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(renderIndices), renderIndices, GL_STATIC_DRAW);

    glBindTexture(GL_TEXTURE_2D, renderTextureId);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    return render_VAO;
}

#endif
