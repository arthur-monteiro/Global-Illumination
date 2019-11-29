#include "GameManager.h"

#include <utility>

GameManager::~GameManager()
{
}

bool GameManager::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkQueue graphicsQueue, std::vector<Image*> swapChainImages)
{
	m_loadingManager.initialize(device, physicalDevice, surface, graphicsQueue, std::move(swapChainImages));
    m_sceneLoadingThread = std::thread(&SceneManager::load, m_gameManager);
    m_sceneLoadingThread.detach();

	return true;
}

void GameManager::submit(VkDevice device, VkQueue graphicsQueue, uint32_t swapChainImageIndex, Semaphore imageAvailableSemaphore)
{
    m_loadingManager.submit(device, graphicsQueue, swapChainImageIndex, imageAvailableSemaphore);
}

void GameManager::resize(VkDevice device, VkPhysicalDevice physicalDevice, std::vector<Image*> swapChainImages)
{
	m_loadingManager.resize(device, physicalDevice, swapChainImages);
}

void GameManager::cleanup(VkDevice device)
{
	m_loadingManager.cleanup(device);
}

VkSemaphore GameManager::getLastRenderFinishedSemaphore()
{
    return m_loadingManager.getLastRenderFinishedSemaphore();
}
