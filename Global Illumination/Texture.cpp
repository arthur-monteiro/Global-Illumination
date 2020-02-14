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

void Texture::createFromPixels(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
	VkQueue graphicsQueue, VkExtent3D extent, VkFormat format, unsigned char* pixels)
{
	m_image.createFromPixels(device, physicalDevice, commandPool, graphicsQueue, extent, format, pixels);
}

bool Texture::createFromFile(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::string filename)
{
	m_image.createFromFile(device, physicalDevice, commandPool, graphicsQueue, filename);

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
