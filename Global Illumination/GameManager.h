#pragma once

#include <thread>

#include "LoadingManager.h"
#include "SceneManager.h"

class GameManagerT
{
public:
	GameManagerT() {}
	~GameManagerT();

	bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkQueue graphicsQueue, std::vector<Image*> swapChainImages);
	void submit(VkDevice device, VkQueue graphicsQueue, uint32_t swapChainImageIndex, Semaphore imageAvailableSemaphore);
	void resize(VkDevice device, VkPhysicalDevice physicalDevice, std::vector<Image*> swapChainImages);

	void cleanup(VkDevice device);

// Getter
public:
    VkSemaphore getLastRenderFinishedSemaphore();

private:
	LoadingManager m_loadingManager;
	SceneManager m_gameManager;
    std::thread m_sceneLoadingThread;
};

