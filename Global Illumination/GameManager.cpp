#include "GameManager.h"

GameManager::~GameManager()
{
}

bool GameManager::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkQueue graphicsQueue, std::vector<Image*> swapChainImages)
{
    m_mutexGraphicsQueue = new std::mutex();

	m_loadingManager.initialize(device, physicalDevice, surface, graphicsQueue, std::move(swapChainImages));
    m_sceneLoadingThread = std::thread(&SceneManager::load, &m_sceneManager, device, physicalDevice, graphicsQueue, surface, m_mutexGraphicsQueue);

	return true;
}

void GameManager::submit(VkDevice device, VkQueue graphicsQueue, uint32_t swapChainImageIndex, Semaphore imageAvailableSemaphore)
{
    if(m_gameState == GAME_STATE::LOADING)
    {
        m_mutexGraphicsQueue->lock();
        m_loadingManager.submit(device, graphicsQueue, swapChainImageIndex, imageAvailableSemaphore);
        m_mutexGraphicsQueue->unlock();

        if(m_sceneManager.getLoadingState() == 1.0f)
        {
            m_gameState = GAME_STATE::RUNNING;
            m_sceneLoadingThread.join();
        }
    }
    else if(m_gameState == GAME_STATE::RUNNING)
    {
        std::cout << "Running \n";
    }
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
