#include "SystemManager.h"

SystemManager::~SystemManager()
{

}

bool SystemManager::initialize()
{
	int width = 1280;
	int height = 720;
	std::string appName = "Global Illumination Demo";

	m_windowManager.initialize(appName, width, height, this, windowResizeCallback);
	m_vulkan.initialize(m_windowManager.getWindow());

	m_swapChain.initialize(m_vulkan.getDevice(), m_vulkan.getPhysicalDevice(), m_vulkan.getSurface(), m_windowManager.getWindow());

	m_sceneManager.initialize(m_vulkan.getDevice(), m_vulkan.getPhysicalDevice(), m_vulkan.getSurface(), m_swapChain.getImages());

	return true;
}

bool SystemManager::run()
{
    while (!glfwWindowShouldClose(m_windowManager.getWindow()))
    {
        glfwPollEvents();

        uint32_t swapChainImageIndex = m_swapChain.getCurrentImage(m_vulkan.getDevice());
        m_sceneManager.submit(m_vulkan.getDevice(), m_vulkan.getGraphicsQueue(), swapChainImageIndex, m_swapChain.getImageAvailableSemaphore());

        m_swapChain.present(m_vulkan.getPresentQueue(), m_sceneManager.getLastRenderFinishedSemaphore(), swapChainImageIndex);
    }

	return true;
}

bool SystemManager::cleanup()
{
	
	return true;
}
