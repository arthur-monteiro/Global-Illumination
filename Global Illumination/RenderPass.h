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

	bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, const std::vector<Attachment>& attachments, std::vector<VkExtent2D> extents);
	bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, const std::vector<Attachment>& attachments, std::vector<Image*> images);

	void fillCommandBuffer(VkDevice device, size_t framebufferID, std::vector<VkClearValue> clearValues, std::vector<Renderer*> renderers, VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT,
		bool endCommandBuffer = true);
	void endCommandBuffer(size_t framebufferID);
	void submit(VkDevice device, VkQueue graphicsQueue, size_t framebufferID, std::vector<Semaphore*> waitSemaphores);
	void resize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, const std::vector<Attachment>& attachments, std::vector<Image*> images);

	void cleanup(VkDevice device, VkCommandPool commandPool);

// Getters
public:
    Semaphore * getRenderCompleteSemaphore() { return &m_renderCompleteSemaphore; }
	std::vector<Image*> getImages(int framebufferID) { return m_framebuffers[framebufferID].getImages(); }
	VkRenderPass getRenderPass() { return m_renderPass; }
	VkFramebuffer getFramebuffer() { return m_framebuffers[0].getFramebuffer(); }
	VkCommandBuffer getCommandBuffer(int framebufferID) { return m_command.getCommandBuffer(framebufferID); }

private:
	VkRenderPass m_renderPass;
	Command m_command;
	std::vector<Framebuffer> m_framebuffers;
	Semaphore m_renderCompleteSemaphore;

private:
	static VkRenderPass createRenderPass(VkDevice device, std::vector<Attachment> attachments);
};

