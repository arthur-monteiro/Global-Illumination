#pragma once

#include "Image.h"
#include "Sampler.h"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

struct TextureLayout
{
	VkShaderStageFlags accessibility;
	uint32_t binding;
};

class Texture
{
public:
	Texture() = default;
	~Texture();

	void create(VkDevice device, VkPhysicalDevice physicalDevice, VkExtent2D extent, VkImageUsageFlags usage, VkFormat format, VkSampleCountFlagBits sampleCount, VkImageAspectFlags aspect);
	void createFromImage(VkDevice device, Image* image);
	void createFromPixels(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkExtent3D extent, VkFormat format, unsigned char* pixels);
	bool createFromFile(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::string filename);
	void createSampler(VkDevice device, VkSamplerAddressMode addressMode, float mipLevels, VkFilter filter);

	void setImageLayout(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImageLayout newLayout, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage);

	void cleanup(VkDevice device);

// Getters
public:
	Image* getImage() { return m_imagePtr == nullptr ? &m_image : m_imagePtr; }
	VkImageView getImageView() { return m_imagePtr == nullptr ? m_image.getImageView() : m_imagePtr->getImageView(); }
	VkImageLayout getImageLayout() { return m_imagePtr == nullptr ? m_image.getImageLayout() : m_imagePtr->getImageLayout(); }
	VkSampler getSampler() { return m_sampler.getSampler(); }
	uint32_t getMipLevels() { return m_imagePtr == nullptr ? m_image.getMipLevels() : m_imagePtr->getMipLevels(); }

private:
	Image m_image;
	Image* m_imagePtr = nullptr;

	Sampler m_sampler;
};

