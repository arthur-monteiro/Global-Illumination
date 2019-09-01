cmake_minimum_required(VERSION 3.13)
project(DemoVK__1)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Freetype REQUIRED)

add_executable(DemoVK__1 main.cpp Camera.cpp Mesh.cpp Pipeline.cpp RenderPass.cpp System.cpp Text.cpp Vulkan.cpp)

target_include_directories(DemoVK__1 PRIVATE /usr/include/freetype2)
target_link_libraries(DemoVK__1 Vulkan::Vulkan)
target_link_libraries(DemoVK__1 glfw)
target_link_libraries(DemoVK__1 freetype)