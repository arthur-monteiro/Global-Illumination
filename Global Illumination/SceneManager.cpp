#include "SceneManager.h"

#include <utility>

SceneManager::~SceneManager()
{
}

bool SceneManager::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkQueue graphicsQueue, std::vector<Image*> swapChainImages)
{
	m_loadingManager.initialize(device, physicalDevice, surface, graphicsQueue, std::move(swapChainImages));

	return true;
}

void SceneManager::submit(VkDevice device, VkQueue graphicsQueue, uint32_t swapChainImageIndex, Semaphore imageAvailableSemaphore)
{
    m_loadingManager.submit(device, graphicsQueue, swapChainImageIndex, imageAvailableSemaphore);
}

void SceneManager::cleanup(VkDevice device)
{
	m_loadingManager.cleanup(device);
}

VkSemaphore SceneManager::getLastRenderFinishedSemaphore()
{
    return m_loadingManager.getLastRenderFinishedSemaphore();
}
