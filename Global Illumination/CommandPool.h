#pragma once

#include "VulkanHelper.h"

class CommandPool
{
public:
	CommandPool() = default;
	~CommandPool();

	void initializeForGraphicsQueue(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	void initializeForComputeQueue(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

	void cleanup(VkDevice device);

	VkCommandPool getCommandPool() { return m_commandPool; }

private:
	VkCommandPool m_commandPool;
};

