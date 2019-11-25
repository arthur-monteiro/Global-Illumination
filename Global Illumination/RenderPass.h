#pragma once

#include <iostream>

#include "VulkanHelper.h"
#include "Command.h"
#include "Framebuffer.h"
#include "Attachment.h"
#include "Semaphore.h"
#include "Renderer.h"

class RenderPass
{
public:
	RenderPass() = default;
	~RenderPass();

	bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkCommandPool commandPool, const std::vector<Attachment>& attachments, std::vector<VkExtent2D> extents);
	bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkCommandPool commandPool, const std::vector<Attachment>& attachments, std::vector<Image*> images);

	void fillCommandBuffer(VkDevice device, size_t framebufferID, std::vector<VkClearValue> clearValues, std::vector<Renderer*> renderers);
	void submit(VkDevice device, VkQueue graphicsQueue, size_t framebufferID, std::vector<Semaphore> waitSemaphores);
	void recreate(VkDevice device, VkPhysicalDevice physicalDevice, const std::vector<Attachment>& attachments, std::vector<Image*> images);

	void cleanup(VkDevice device);

// Getters
public:
    VkSemaphore getRenderCompleteSemaphore() { return m_renderCompleteSemaphore.getSemaphore(); }

private:
	VkRenderPass m_renderPass;
	Command m_command;
	std::vector<Framebuffer> m_framebuffers;
	Semaphore m_renderCompleteSemaphore;

private:
	static VkRenderPass createRenderPass(VkDevice device, std::vector<Attachment> attachments);
};

