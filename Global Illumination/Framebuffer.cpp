#include "Framebuffer.h"

Framebuffer::~Framebuffer()
{
}

//bool Framebuffer::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkRenderPass renderPass, VkExtent2D extent, std::vector<Attachment> attachments)
//{
//	VkFramebufferCreateInfo framebufferInfo = {};
//	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//	framebufferInfo.renderPass = renderPass;
//	framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
//	framebufferInfo.pAttachments = attachments.data();
//	framebufferInfo.width = extent.width;
//	framebufferInfo.height = extent.height;
//	framebufferInfo.layers = 1;
//
//	return vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_framebuffer) != VK_SUCCESS;
//}

bool Framebuffer::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkRenderPass renderPass, Image* image, std::vector<Attachment> attachments)
{
    m_extent = image->getExtent();

	// Find result image
	VkImageUsageFlagBits resultType = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	int nbImage = 0;
	for (int i(0); i < attachments.size(); ++i)
	{
		if (attachments[i].getUsageType() == (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT))
			resultType = static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);
		else if(attachments[i].getUsageType() == VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT && resultType == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
			resultType = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}
	for (int i(0); i < attachments.size(); ++i)
		if (attachments[i].getUsageType() != resultType)
			nbImage++;

	if (nbImage < attachments.size() - 1)
		throw std::runtime_error("Error : multiple result with single image");

	m_images.resize(nbImage);
	std::vector<VkImageView> imageViewAttachments(attachments.size());

	// Create necessary images
	int currentImage = 0;
	for (int i(0); i < attachments.size(); ++i)
	{
		if (attachments[i].getUsageType() != resultType)
		{
			VkImageAspectFlagBits aspect;
			if (attachments[i].getUsageType() & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
				aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			else
				aspect = VK_IMAGE_ASPECT_DEPTH_BIT;

			m_images[currentImage].create(device, physicalDevice, image->getExtent(), attachments[i].getUsageType(), attachments[i].getFormat(), attachments[i].getSampleCount(), aspect);

			imageViewAttachments[i] = m_images[currentImage].getImageView();

			currentImage++;
		}
		else
		{
			imageViewAttachments[i] = image->getImageView();
		}
	}

	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderPass;
	framebufferInfo.attachmentCount = static_cast<uint32_t>(imageViewAttachments.size());
	framebufferInfo.pAttachments = imageViewAttachments.data();
	framebufferInfo.width = image->getExtent().width;
	framebufferInfo.height = image->getExtent().height;
	framebufferInfo.layers = 1;

	if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_framebuffer) != VK_SUCCESS)
		throw std::runtime_error("Error : create framebuffer");

	return true;
}
