#include "CommandPool.h"

CommandPool::~CommandPool()
{
}

void CommandPool::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	m_commandPool = createCommandPool(device, physicalDevice, surface);
}

void CommandPool::cleanup(VkDevice device)
{
	vkDestroyCommandPool(device, m_commandPool, nullptr);
}
