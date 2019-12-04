#pragma once

#include <array>

#include "VulkanHelper.h"
#include "Renderer.h"
#include "Operation.h"
#include "Semaphore.h"

class Command
{
public:
	Command() {};
	~Command();

	bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	void allocateCommandBuffers(VkDevice device, VkCommandPool commandPool, size_t size);
	void fillCommandBuffer(VkDevice device, size_t commandBufferID, VkRenderPass renderPass, VkFramebuffer framebuffer, VkExtent2D extent, std::vector<VkClearValue> clearValues, 
		std::vector<Renderer*> renderers);
	void fillCommandBuffer(VkDevice device, size_t commandBufferID, std::vector<Operation> operations);
	void submit(VkDevice device, VkQueue graphicsQueue, std::vector<Semaphore*> waitSemaphores, std::vector<VkSemaphore> signalSemaphores, size_t commandBufferID);

	void cleanup(VkDevice device, VkCommandPool commandPool);

// Getter
public:
    VkCommandBuffer getCommandBuffer(size_t id) { return m_commandBuffers[id]; }

private:
	std::vector <VkCommandBuffer> m_commandBuffers;
};

