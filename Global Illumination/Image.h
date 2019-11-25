#pragma once

#include <iostream>

#include "VulkanHelper.h"

class Image
{
public:
	void create(VkDevice device, VkPhysicalDevice physicalDevice, VkExtent2D extent, VkImageUsageFlags usage, VkFormat format, VkSampleCountFlagBits sampleCount, VkImageAspectFlags aspect);
	void createFromImage(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspect, VkExtent2D extent);

	void cleanup(VkDevice device);

	VkImage getImage() { return m_image; }
	VkDeviceMemory getImageMemory() { return m_imageMemory; }
	VkImageView getImageView() { return m_imageView; }
	VkFormat getFormat() { return m_imageFormat; }
	VkSampleCountFlagBits getSampleCount() { return m_sampleCount; }
	VkExtent2D getExtent() { return m_extent; }

private:
	VkImage m_image;
	VkDeviceMemory  m_imageMemory;
	VkImageView m_imageView = VK_NULL_HANDLE;

	VkImageLayout m_imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkFormat m_imageFormat;

	uint32_t m_mipLevels;
	VkExtent2D m_extent;
	VkSampleCountFlagBits m_sampleCount;

private:
	static void createImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, 
		VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, uint32_t arrayLayers, VkImageCreateFlags flags, VkImageLayout initialLayout, 
		VkImage& image, VkDeviceMemory& imageMemory);
	static VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, VkImageViewType viewType);

};