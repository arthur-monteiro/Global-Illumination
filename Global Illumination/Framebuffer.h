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

	VkFramebuffer getFramebuffer() { return m_framebuffer; }
	VkExtent2D getExtent() { return m_extent; }

private:
	VkFramebuffer m_framebuffer;
	VkExtent2D m_extent;
	std::vector<Image> m_images;
};

