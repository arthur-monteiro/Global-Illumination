#pragma once

#include "Vulkan.h"
#include "Sampler.h"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

class Image
{
public:
	void loadTextureFromFile(Vulkan* vk, std::string path);
	void create(Vulkan* vk, VkExtent2D extent, VkImageUsageFlags usage, VkFormat format, VkSampleCountFlagBits sampleCount, VkImageAspectFlags aspect);
	void transitionImageLayout(Vulkan* vk, VkImageLayout finalLayout);
	void createTextureSampler(Vulkan* vk, VkSamplerAddressMode addressMode);

	void cleanup(VkDevice device);

	VkImage getImage() { return m_image; }
	VkDeviceMemory getImageMemory() { return m_imageMemory; }
	VkImageView getImageView() { return m_imageView; }
	VkSampler getSampler() { return m_textureSampler.getSampler(); }

private:
	VkImage m_image;
	VkDeviceMemory  m_imageMemory;
	VkImageView m_imageView = VK_NULL_HANDLE;

	VkImageLayout m_imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkFormat m_imageFormat;

	uint32_t m_mipLevels;

	Sampler m_textureSampler;
};

