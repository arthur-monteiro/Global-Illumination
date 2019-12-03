#include "Texture.h"

Texture::~Texture()
{
}

void Texture::create(VkDevice device, VkPhysicalDevice physicalDevice, VkExtent2D extent, VkImageUsageFlags usage, VkFormat format, VkSampleCountFlagBits sampleCount, VkImageAspectFlags aspect)
{
	m_image.create(device, physicalDevice, extent, usage, format, sampleCount, aspect);
}

void Texture::createFromImage(VkDevice device, Image* image)
{
	m_imagePtr = image;
}

bool Texture::createFromFile(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::string filename)
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	texChannels = 4;

	if (!pixels)
		throw std::runtime_error("Error : loading image " + filename);

	m_image.createFromPixels(device, physicalDevice, commandPool, graphicsQueue, { static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), static_cast<uint32_t>(texChannels) }, pixels);
	stbi_image_free(pixels);

	return true;
}

void Texture::createSampler(VkDevice device, VkSamplerAddressMode addressMode, float mipLevels, VkFilter filter)
{
	m_sampler.initialize(device, addressMode, mipLevels, filter);
}

void Texture::setImageLayout(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImageLayout newLayout, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage)
{
	m_imagePtr == nullptr ?
		m_image.setImageLayout(device, commandPool, graphicsQueue, newLayout, sourceStage, destinationStage) :
		m_imagePtr->setImageLayout(device, commandPool, graphicsQueue, newLayout, sourceStage, destinationStage);
}

void Texture::cleanup(VkDevice device)
{
	m_image.cleanup(device);
	m_sampler.cleanup(device);
}
