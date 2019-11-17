cmake_minimum_required(VERSION 3.15)
project(Global_Illumination)

set(CMAKE_CXX_STANDARD 14)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Freetype REQUIRED)

add_executable(Global_Illumination
        Attachment.cpp
        Attachment.h
        Command.cpp
        Command.h
        Framebuffer.cpp
        Framebuffer.h
        Image.cpp
        Image.h
        LoadingManager.cpp
        LoadingManager.h
        main.cpp
        RenderPass.cpp
        RenderPass.h
        SceneManager.cpp
        SceneManager.h
        SwapChain.cpp
        SwapChain.h
        SystemManager.cpp
        SystemManager.h
        Vulkan.cpp
        Vulkan.h
        VulkanHelper.cpp
        VulkanHelper.h
        WindowManager.cpp
        WindowManager.h Semaphore.cpp Semaphore.h SceneElement.cpp SceneElement.h Model.cpp Model.h Mesh.cpp Mesh.h Mesh2D.cpp Mesh2D.h)

target_include_directories(Global_Illumination PRIVATE /usr/include/freetype2)
target_link_libraries(Global_Illumination Vulkan::Vulkan)
target_link_libraries(Global_Illumination glfw)
target_link_libraries(Global_Illumination freetype)