cmake_minimum_required(VERSION 3.16)
project(DesktopSetup3D LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

add_executable(DesktopSetup3D
    src/main.cpp
    src/Shader.cpp
    src/Texture.cpp
    src/Mesh.cpp
    src/SceneObjects.cpp
)

target_include_directories(DesktopSetup3D PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(DesktopSetup3D PRIVATE
    OpenGL::GL
    glfw
    GLEW::GLEW
    glm::glm
)

# Copy shaders folder to build output directory so relative paths work from anywhere
add_custom_command(TARGET DesktopSetup3D POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:DesktopSetup3D>/shaders
)

