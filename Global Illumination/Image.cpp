#include "Image.h"

void Image::loadTextureFromFile(Vulkan* vk, std::string path)
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = static_cast<uint64_t>(texWidth * texHeight * 4);
	m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	if (!pixels)
		throw std::runtime_error("Erreur : chargement de l'image " + path + " !");

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	vk->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	uint8_t* data;
	vkMapMemory(vk->getDevice(), stagingBufferMemory, 0, imageSize, 0, (void**)& data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(vk->getDevice(), stagingBufferMemory);

	stbi_image_free(pixels);

	vk->createImage(texWidth, texHeight, m_mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, 0, VK_IMAGE_LAYOUT_PREINITIALIZED,
		m_image, m_imageMemory);

	vk->transitionImageLayout(m_image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_mipLevels, 1);
	vk->copyBufferToImage(stagingBuffer, m_image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 0);
	//vk->transitionImageLayout(m_textureImage[m_textureImage.size() - 1], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_mipLevels);

	vk->generateMipmaps(m_image, VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, m_mipLevels, 0);

	vkDestroyBuffer(vk->getDevice(), stagingBuffer, nullptr);
	vkFreeMemory(vk->getDevice(), stagingBufferMemory, nullptr);

	m_imageView = vk->createImageView(m_image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, m_mipLevels, VK_IMAGE_VIEW_TYPE_2D);
}

void Image::create(Vulkan* vk, VkExtent2D extent, VkImageUsageFlags usage, VkFormat format, VkImageLayout finalLayout)
{
	vk->createImage(extent.width, extent.height, 1, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL,
		usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, 0, VK_IMAGE_LAYOUT_UNDEFINED,
		m_image, m_imageMemory);
	vk->transitionImageLayout(m_image, format, VK_IMAGE_LAYOUT_UNDEFINED, finalLayout, 1, 1);
	
	m_imageView = vk->createImageView(m_image, format, VK_IMAGE_ASPECT_COLOR_BIT, 1, VK_IMAGE_VIEW_TYPE_2D);
}

void Image::createTextureSampler(Vulkan* vk, VkSamplerAddressMode addressMode)
{
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;

	samplerInfo.addressModeU = addressMode;
	samplerInfo.addressModeV = addressMode;
	samplerInfo.addressModeW = addressMode;

	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 16;

	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast<float>(m_mipLevels);

	if (vkCreateSampler(vk->getDevice(), &samplerInfo, nullptr, &m_textureSampler) != VK_SUCCESS)
		throw std::runtime_error("Error : texture sampler creation");
}
