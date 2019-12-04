#include "CommandPool.h"

CommandPool::~CommandPool()
{
}

void CommandPool::initializeForGraphicsQueue(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice, surface);

	m_commandPool = createCommandPool(device, physicalDevice, surface, queueFamilyIndices.graphicsFamily);
}

void CommandPool::initializeForComputeQueue(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice, surface);

	m_commandPool = createCommandPool(device, physicalDevice, surface, queueFamilyIndices.computeFamily);
}

void CommandPool::cleanup(VkDevice device)
{
	vkDestroyCommandPool(device, m_commandPool, nullptr);
}
