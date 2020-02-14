#pragma once

#include <thread>
#include <mutex>

#include "LoadingManager.h"
#include "SceneManager.h"

enum GAME_STATE
{
    LOADING,
    RUNNING
};

class GameManager
{
public:
	GameManager() = default;
	~GameManager();

	bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkQueue graphicsQueue, std::mutex* graphicsQueueMutex, 
		VkQueue computeQueue, std::mutex* computeQueueMutex, std::vector<Image*> swapChainImages, bool rayTracingAvailable);
	void submit(VkDevice device, VkPhysicalDevice physicalDevice, GLFWwindow* window, VkQueue graphicsQueue, std::mutex* graphicsQueueMutex,
		VkQueue computeQueue, std::mutex* computeQueueMutex, uint32_t swapChainImageIndex, Semaphore* imageAvailableSemaphore);
	void resize(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkQueue computeQueue, std::vector<Image*> swapChainImages);

	void cleanup(VkDevice device);

// Getter
public:
    VkSemaphore getLastRenderFinishedSemaphore();

private:
	LoadingManager m_loadingManager;
	SceneManager m_sceneManager;

    std::thread m_sceneLoadingThread;

    GAME_STATE m_gameState = GAME_STATE::LOADING;
};

