#pragma once

#include "VulkanHelper.h"
#include "Attachment.h"

class Framebuffer
{
public:
	Framebuffer() {}
	~Framebuffer();

	bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkRenderPass renderPass, VkExtent2D extent, std::vector<Attachment> attachments);
	bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkRenderPass renderPass, Image * image, std::vector<Attachment> attachments);

private:
	VkFramebuffer m_framebuffer;
	std::vector<Image> m_images;
};

