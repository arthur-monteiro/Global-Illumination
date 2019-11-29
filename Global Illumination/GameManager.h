#pragma once

#include <thread>
#include <mutex>

#include "LoadingManager.h"
#include "SceneManager.h"

enum GAME_STATE {
    LOADING,
    RUNNING
};

class GameManager
{
public:
	GameManager() {}
	~GameManager();

	bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkQueue graphicsQueue, std::vector<Image*> swapChainImages);
	void submit(VkDevice device, VkQueue graphicsQueue, uint32_t swapChainImageIndex, Semaphore imageAvailableSemaphore);
	void resize(VkDevice device, VkPhysicalDevice physicalDevice, std::vector<Image*> swapChainImages);

	void cleanup(VkDevice device);

// Getter
public:
    VkSemaphore getLastRenderFinishedSemaphore();

private:
	LoadingManager m_loadingManager;
	SceneManager m_sceneManager;

    std::thread m_sceneLoadingThread;
    std::mutex * m_mutexGraphicsQueue;

    GAME_STATE m_gameState = GAME_STATE::LOADING;
};

