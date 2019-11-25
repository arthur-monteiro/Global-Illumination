#pragma once

#include <array>

#include "VulkanHelper.h"
#include "Renderer.h"

class Command
{
public:
	Command() {};
	~Command();

	bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	void allocateCommandBuffers(VkDevice device, VkCommandPool commandPool, size_t size);
	void fillCommandBuffer(VkDevice device, size_t commandBufferID, VkRenderPass renderPass, VkFramebuffer framebuffer, VkExtent2D extent, std::vector<VkClearValue> clearValues, 
		std::vector<Renderer*> renderers);

	void cleanup(VkDevice device);

// Getter
public:
    VkCommandBuffer getCommandBuffer(size_t id) { return m_commandBuffers[id]; }

private:
	std::vector <VkCommandBuffer> m_commandBuffers;
};

