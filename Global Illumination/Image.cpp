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
	m_mipLevels = 1;
}

void Image::createFromImage(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspect, VkExtent2D extent)
{
	m_image = image;
	m_imageMemory = VK_NULL_HANDLE;
	m_imageFormat = format;
	m_imageView = createImageView(device, m_image, format, aspect, 1, VK_IMAGE_VIEW_TYPE_2D);
	m_extent = extent;
	m_mipLevels = 1;
}

void Image::createFromPixels(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkExtent3D extent, VkFormat format,
	unsigned char* pixels)
{
	//m_imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
	m_imageFormat = format;

	VkDeviceSize imageSize = static_cast<uint64_t>(extent.width * extent.height * extent.depth);
	m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(extent.width, extent.height)))) + 1;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(device, physicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	uint8_t* data;
	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, (void**)& data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(device, stagingBufferMemory);

	createImage(device, physicalDevice, extent.width, extent.height, m_mipLevels, VK_SAMPLE_COUNT_1_BIT, m_imageFormat, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, 0,
		VK_IMAGE_LAYOUT_PREINITIALIZED, m_image, m_imageMemory);

	transitionImageLayout(device, commandPool, graphicsQueue, m_image, m_imageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_mipLevels, 1,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
	copyBufferToImage(device, commandPool, graphicsQueue, stagingBuffer, m_image, extent.width, extent.height, 0);
	//vk->transitionImageLayout(m_textureImage[m_textureImage.size() - 1], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_mipLevels);

	generateMipmaps(device, physicalDevice, commandPool, graphicsQueue, m_image, m_imageFormat, extent.width, extent.height, m_mipLevels, 0);
	m_imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

	m_imageView = createImageView(device, m_image, m_imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, m_mipLevels, VK_IMAGE_VIEW_TYPE_2D);
}

void Image::createFromBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
	VkQueue graphicsQueue, VkExtent3D extent, VkFormat format, VkBuffer buffer)
{
	m_imageFormat = format;

	createImage(device, physicalDevice, extent.width, extent.height, m_mipLevels, VK_SAMPLE_COUNT_1_BIT, m_imageFormat, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, 0,
		VK_IMAGE_LAYOUT_PREINITIALIZED, m_image, m_imageMemory);

	transitionImageLayout(device, commandPool, graphicsQueue, m_image, m_imageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_mipLevels, 1,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

	copyBufferToImage(device, commandPool, graphicsQueue, buffer, m_image, extent.width, extent.height, 0);

	generateMipmaps(device, physicalDevice, commandPool, graphicsQueue, m_image, m_imageFormat, extent.width, extent.height, m_mipLevels, 0);
	m_imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	m_imageView = createImageView(device, m_image, m_imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, m_mipLevels, VK_IMAGE_VIEW_TYPE_2D);

	m_imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

void Image::createFromFile(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::string filename)
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	texChannels = 4;

	if (!pixels)
		throw std::runtime_error("Error : loading image " + filename);

	createFromPixels(device, physicalDevice, commandPool, graphicsQueue, { static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), static_cast<uint32_t>(texChannels) }, VK_FORMAT_R8G8B8A8_UNORM,
		pixels);
	stbi_image_free(pixels);

	m_extent.width = texWidth;
	m_extent.height = texHeight;
	m_sampleCount = VK_SAMPLE_COUNT_1_BIT;
	m_imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
}

void Image::createCubeMapFromImages(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::array<Image*, 6> images)
{
	m_mipLevels = images[0]->getMipLevels();
	m_imageFormat = images[0]->getFormat();
	m_sampleCount = images[0]->getSampleCount();
	m_extent = images[0]->getExtent();

	createImage(device, physicalDevice, m_extent.width, m_extent.height, m_mipLevels, m_sampleCount, m_imageFormat, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 6, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
		m_image, m_imageMemory);
	transitionImageLayout(device, commandPool, graphicsQueue, m_image, m_imageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_mipLevels, 6,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

	for (int i = 0; i < images.size(); ++i)
	{
		transitionImageLayout(device, commandPool, graphicsQueue, images[i]->getImage(), m_imageFormat, images[i]->getImageLayout(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1, 1,
			VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		copyImage(device, commandPool, graphicsQueue, images[i]->getImage(), m_image, m_extent.width, m_extent.height, i, 0);

		generateMipmaps(device, physicalDevice, commandPool, graphicsQueue, m_image, m_imageFormat, m_extent.width, m_extent.height, m_mipLevels, i);
	}

	m_imageView = createImageView(device, m_image, m_imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, m_mipLevels, VK_IMAGE_VIEW_TYPE_CUBE);
	m_imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

void Image::setImageLayout(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImageLayout newLayout, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage)
{
	transitionImageLayout(device, commandPool, graphicsQueue, m_image, m_imageFormat, m_imageLayout, newLayout, m_mipLevels, 1, sourceStage, destinationStage);
	m_imageLayout = newLayout;
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

	if (allocInfo.memoryTypeIndex < 0)
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

void Image::transitionImageLayout(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
	uint32_t mipLevels, uint32_t arrayLayers, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage)
{
	for (uint32_t arrayLayer = 0; arrayLayer < arrayLayers; arrayLayer++)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

		transitionImageLayoutUsingCommandBuffer(commandBuffer, image, format, oldLayout, newLayout, mipLevels,
			sourceStage, destinationStage, arrayLayer);

		endSingleTimeCommands(device, graphicsQueue, commandBuffer, commandPool);
	}
}

void Image::copyBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t baseArrayLayer)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = baseArrayLayer;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent =
	{
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(
		commandBuffer,
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	endSingleTimeCommands(device, graphicsQueue, commandBuffer, commandPool);
}

void Image::generateMipmaps(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, VkFormat imageFormat, int32_t texWidth,
	int32_t texHeight, uint32_t mipLevels, uint32_t baseArrayLayer)
{
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
		throw std::runtime_error("Error : format non supported for mipmap generation");

	VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = baseArrayLayer;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	for (uint32_t i = 1; i < mipLevels; i++)
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		VkImageBlit blit = {};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = baseArrayLayer;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = baseArrayLayer;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(commandBuffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	endSingleTimeCommands(device, graphicsQueue, commandBuffer, commandPool);
}

void Image::transitionImageLayoutUsingCommandBuffer(VkCommandBuffer commandBuffer,
	VkImage image, VkFormat format, VkImageLayout oldLayout,
	VkImageLayout newLayout, uint32_t mipLevels,
	VkPipelineStageFlags sourceStage,
	VkPipelineStageFlags destinationStage, uint32_t arrayLayer)
{
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = arrayLayer;
	barrier.subresourceRange.layerCount = 1;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL || hasDepthComponent(format))
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (hasStencilComponent(format))
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	else
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	switch (oldLayout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		barrier.srcAccessMask = 0;
		break;

	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		barrier.srcAccessMask = 0;
		break;

	case VK_IMAGE_LAYOUT_GENERAL:
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;

	default:
		throw std::runtime_error("Error : image layout transition not supported");
		break;
	}

	switch (newLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		barrier.dstAccessMask = barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		if (barrier.srcAccessMask == 0)
		{
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		}
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_GENERAL:
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		barrier.dstAccessMask = 0;
		break;

	default:
		throw std::runtime_error("Error : image layout transition not supported");
		break;
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
}