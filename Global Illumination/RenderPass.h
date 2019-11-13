#pragma once

#include <iostream>

#include "VulkanHelper.h"
#include "Command.h"
#include "Framebuffer.h"
#include "Attachment.h"
#include "Semaphore.h"

class RenderPass
{
public:
	RenderPass() {}
	~RenderPass();

	bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const std::vector<Attachment>& attachments, std::vector<VkExtent2D> extents);
	bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const std::vector<Attachment>& attachments, std::vector<Image*> images);

	void fillCommandBuffer(VkDevice device, size_t framebufferID, std::vector<VkClearValue> clearValues);

	void submit(VkDevice device, VkQueue graphicsQueue, size_t framebufferID, std::vector<Semaphore> waitSemaphores);

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

