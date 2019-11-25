#include "Image.h"

void Image::create(VkDevice device, VkPhysicalDevice physicalDevice, VkExtent2D extent, VkImageUsageFlags usage, VkFormat format, VkSampleCountFlagBits sampleCount, VkImageAspectFlags aspect)
{
	m_imageFormat = format;
	m_extent = extent;
	m_sampleCount = sampleCount;

	createImage(device, physicalDevice, extent.width, extent.height, 1, sampleCount, format, VK_IMAGE_TILING_OPTIMAL,
		usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, 0, VK_IMAGE_LAYOUT_UNDEFINED,
		m_image, m_imageMemory);
	m_imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	m_imageView = createImageView(device, m_image, format, aspect, 1, VK_IMAGE_VIEW_TYPE_2D);
}

void Image::createFromImage(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspect, VkExtent2D extent)
{
	m_image = image;
	m_imageMemory = VK_NULL_HANDLE;
	m_imageFormat = format;
	m_imageView = createImageView(device, m_image, format, aspect, 1, VK_IMAGE_VIEW_TYPE_2D);
	m_extent = extent;
}

void Image::cleanup(VkDevice device)
{
	vkDestroyImageView(device, m_imageView, nullptr);
	vkDestroyImage(device, m_image, nullptr);
	vkFreeMemory(device, m_imageMemory, nullptr);
}

void Image::createImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
	VkImageUsageFlags usage, VkMemoryPropertyFlags properties, uint32_t arrayLayers, VkImageCreateFlags flags, VkImageLayout initialLayout, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = arrayLayers;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = initialLayout;
	imageInfo.usage = usage;
	imageInfo.samples = numSamples;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.flags = flags;

	if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
		throw std::runtime_error("Error : create image");

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

	if(allocInfo.memoryTypeIndex < 0)
		throw std::runtime_error("Error : no memory type found");

	if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate image memory!");

	vkBindImageMemory(device, image, imageMemory, 0);
}

VkImageView Image::createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, VkImageViewType viewType)
{
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = viewType;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
		throw std::runtime_error("Error : create image view");

	return imageView;
}
