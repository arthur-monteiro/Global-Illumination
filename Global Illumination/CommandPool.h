#pragma once

#include "VulkanHelper.h"

class CommandPool
{
public:
	CommandPool() = default;
	~CommandPool();

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

	void cleanup(VkDevice device);

	VkCommandPool getCommandPool() { return m_commandPool; }

private:
	VkCommandPool m_commandPool;
};

