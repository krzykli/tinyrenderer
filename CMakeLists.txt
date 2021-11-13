project(tinyrenderer)
set (CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Release)
cmake_minimum_required(VERSION 3.19)

find_package(OpenGL REQUIRED)

include_directories("/usr/local/Cellar/glfw/3.3.4/include")
include_directories("/usr/local/Cellar/glew/2.2.0_1/include")
include_directories("/usr/local/Cellar/glm/0.9.9.8/include")
include_directories("./thirdparty/imgui")
include_directories("./thirdparty/imguiFileDialog")
include_directories(".")

file(GLOB RENDERER_SRC *.cpp)
file(GLOB IMGUI_SRC ./thirdparty/imgui/*.cpp)
file(GLOB IMGUIFILEDIALOG_SRC ./thirdparty/imguiFileDialog/*.cpp)
add_executable(tinyrenderer ${IMGUI_SRC} ${IMGUIFILEDIALOG_SRC} ${RENDERER_SRC})

target_link_libraries(tinyrenderer
    ${OPENGL_LIBRARY}
    "/usr/local/Cellar/glfw/3.3.4/lib/libglfw.dylib"
    "/usr/local/Cellar/glew/2.2.0_1/lib/libGLEW.dylib"
)

add_compile_options("-Wall -D")

