#pragma once

#include "RenderPass.h"

class LoadingManager
{
public:
	LoadingManager() {}
	~LoadingManager();

	bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, std::vector<Image*> swapChainImages);
	bool submit(VkDevice device, VkQueue graphicsQueue, uint32_t swapChainImageIndex, const Semaphore& imageAvailableSemaphore);

// Getters
public:
    VkSemaphore getLastRenderFinishedSemaphore();

private:
    RenderPass m_mainRenderPass;
};

