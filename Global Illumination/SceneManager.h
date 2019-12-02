#pragma once

#include <mutex>

#include "ModelPBR.h"
#include "CommandPool.h"
#include "DescriptorPool.h"
#include "GBuffer.h"

class SceneManager
{
public:
    SceneManager() = default;
    ~SceneManager();

    void load(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkSurfaceKHR surface, std::mutex * graphicsQueueMutex,
              std::vector<Image*> swapChainImages);

    float getLoadingState() { return m_loadingState; }

private:
    float m_loadingState = 0.0f;

    CommandPool m_commandPool;
	DescriptorPool m_descriptorPool;

    ModelPBR m_model;
    GBuffer m_gbuffer;
};