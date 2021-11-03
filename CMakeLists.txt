project(tinyrenderer)
cmake_minimum_required(VERSION 3.19)

find_package(OpenGL REQUIRED)

include_directories("/usr/local/Cellar/glfw/3.3.4/include")
include_directories("/usr/local/Cellar/glew/2.2.0_1/include")
include_directories("./thirdparty/imgui")

file(GLOB RENDERER_SRC *.cpp)
file(GLOB IMGUI_SRC ./thirdparty/imgui/*.cpp)
add_executable(tinyrenderer ${IMGUI_SRC} ${RENDERER_SRC})

target_link_libraries(tinyrenderer
    ${OPENGL_LIBRARY}
    "/usr/local/Cellar/glfw/3.3.4/lib/libglfw.dylib"
    "/usr/local/Cellar/glew/2.2.0_1/lib/libGLEW.dylib"
)

add_compile_options("-Wall -D")

add_definitions(
    -DCMAKE_BUILD_TYPE=Debug
    -DCMAKE_EXPORT_COMPILE_COMMANDS=YES
)
