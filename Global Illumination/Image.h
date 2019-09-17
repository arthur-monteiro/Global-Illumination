#pragma once

#include "Vulkan.h"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

class Image
{
public:
	void loadTextureFromFile(Vulkan* vk, std::string path);
	void create(Vulkan* vk, VkExtent2D extent, VkImageUsageFlags usage);
	void createTextureSampler(Vulkan* vk, VkSamplerAddressMode addressMode);

	VkImage getImage() { return m_image; }
	VkDeviceMemory getImageMemory() { return m_imageMemory; }
	VkImageView getImageView() { return m_imageView; }
	VkSampler getSampler() { return m_textureSampler; }

private:
	VkImage m_image;
	VkDeviceMemory  m_imageMemory;
	VkImageView m_imageView = VK_NULL_HANDLE;

	uint32_t m_mipLevels;

	VkSampler m_textureSampler = NULL;
};

