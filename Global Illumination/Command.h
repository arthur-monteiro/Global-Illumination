#pragma once

#include <array>

#include "VulkanHelper.h"

class Command
{
public:
	Command() {};
	~Command();

	bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	void allocateCommandBuffers(VkDevice device, size_t size);
	void fillCommandBuffer(VkDevice device, size_t commandBufferID, VkRenderPass renderPass, VkFramebuffer framebuffer, VkExtent2D extent, std::vector<VkClearValue> clearValues);

// Getter
public:
    VkCommandBuffer getCommandBuffer(size_t id) { return m_commandBuffers[id]; }

private:
	VkCommandPool m_commandPool;
	std::vector <VkCommandBuffer> m_commandBuffers;

private:
	static VkCommandPool createCommandPool(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
};

