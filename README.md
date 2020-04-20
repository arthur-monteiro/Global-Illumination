# Global Illumination

This project is a global illumination technical demo using Vulkan

![Screenshot](https://github.com/arthur-monteiro/Global-Illumination/blob/master/Pictures%20description/Capture.PNG)
![Screenshot](https://github.com/arthur-monteiro/Global-Illumination/blob/master/Pictures%20description/Capture_2.PNG)

This demo integrates the following techniques:
* Physically based rendering
* Multisampling anti-aliasing (MSAA)
* Cascaded Shadow Mapping
* Shadows with ray tracing
* Bloom
* Tone mapping
* Screen space reflections (SSR)
* Screen space ambient occlusion (SSAO)
* Text rendering

## Installation

### Windows

This projet uses Visual Studio 2019 on Windows.
You need to install some libraries and set an environnement variable:
* VULKAN_SDK : folder of your vulkan installation (for example C:\VulkanSDK\1.1.108.0)
* STB_IMAGE_PATH : path to a folder that includes a folder name "stb_image" in which is located the file "stb_image.h
* FREETYPE_PATH : path to your freetype installation. It must include the folders "include" and "lib"
* GLM_PATH : path to your GLM installation. It must include a folder "glm" in which the glm code is located
* GLFW_PATH : path to your GLFW installation. It msust include a folder "include" and "lib-vc2019"
