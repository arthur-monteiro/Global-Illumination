cmake_minimum_required(VERSION 3.15)
project(Global_Illumination)

set(CMAKE_CXX_STANDARD 14)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Freetype REQUIRED)
find_package (Threads REQUIRED)

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
        GameManager.cpp
        GameManager.h
        SwapChain.cpp
        SwapChain.h
        SystemManager.cpp
        SystemManager.h
        Vulkan.cpp
        Vulkan.h
        VulkanHelper.cpp
        VulkanHelper.h
        WindowManager.cpp
        WindowManager.h
        Semaphore.cpp
        Semaphore.h
        Model.cpp
        Model.h
        Mesh.cpp
        Mesh.h
        CommandPool.cpp
        CommandPool.h
        DescriptorPool.cpp
        DescriptorPool.h
        Model2DTextured.cpp
        Model2DTextured.h
        Texture.cpp
        Texture.h
        Sampler.cpp
        Sampler.h
        Renderer.cpp
        Renderer.h
        UniformBufferObject.cpp
        UniformBufferObject.h
        Pipeline.cpp
        Pipeline.h SceneManager.cpp SceneManager.h ModelPBR.cpp ModelPBR.h GBuffer.cpp GBuffer.h)

target_include_directories(Global_Illumination PRIVATE /usr/include/freetype2)
target_link_libraries(Global_Illumination Vulkan::Vulkan)
target_link_libraries(Global_Illumination glfw)
target_link_libraries(Global_Illumination freetype)
target_link_libraries (Global_Illumination Threads::Threads)