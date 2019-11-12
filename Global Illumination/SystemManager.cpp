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

	return true;
}

bool SystemManager::run()
{
	return true;
}

bool SystemManager::cleanup()
{
	
	return true;
}
