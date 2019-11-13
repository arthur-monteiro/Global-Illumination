#pragma once

#include "LoadingManager.h"

class SceneManager
{
public:
	SceneManager() {}
	~SceneManager();

	bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, std::vector<Image*> swapChainImages);
	void submit(VkDevice device, VkQueue graphicsQueue, uint32_t swapChainImageIndex, Semaphore imageAvailableSemaphore);

// Getter
public:
    VkSemaphore getLastRenderFinishedSemaphore();

private:
	LoadingManager m_loadingManager;
};

