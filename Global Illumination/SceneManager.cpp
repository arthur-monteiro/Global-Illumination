#include "SceneManager.h"

SceneManager::~SceneManager()
{

}

void SceneManager::load(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkSurfaceKHR surface,
        std::mutex * graphicsQueueMutex,  std::vector<Image*> swapChainImages)
{
    m_commandPool.initialize(device, physicalDevice, surface);
    m_model.loadFromFile(device, physicalDevice, m_commandPool.getCommandPool(), graphicsQueue, graphicsQueueMutex,
            "Models/sponza/sponza.obj", "Models/sponza");
    m_gbuffer.initialize(device, physicalDevice, surface, m_commandPool.getCommandPool(), swapChainImages[0]->getExtent());

    m_loadingState = 1.0f;
}
