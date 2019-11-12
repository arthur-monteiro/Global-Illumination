#pragma once

#include <iostream>

#include "VulkanHelper.h"
#include "Command.h"
#include "Framebuffer.h"
#include "Attachment.h"

class RenderPass
{
public:
	RenderPass() {}
	~RenderPass();

	bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, std::vector<Attachment> attachments, std::vector<VkExtent2D> extents);
	bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, std::vector<Attachment> attachments, std::vector<Image*> images);

	void submit(VkDevice device);

private:
	VkRenderPass m_renderPass;
	Command m_command;
	std::vector<Framebuffer> m_framebuffers;

private:
	static VkRenderPass createRenderPass(VkDevice device, std::vector<Attachment> attachments);
};

