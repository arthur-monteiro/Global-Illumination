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

	bool createFromFile(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::string filename);
	void createSampler(VkDevice device, VkSamplerAddressMode addressMode, float mipLevels, VkFilter filter);

	void cleanup(VkDevice device);

// Getters
public:
	VkImageView getImageView() { return m_image.getImageView(); }
	VkImageLayout getImageLayout() { return m_image.getImageLayout(); }
	VkSampler getSampler() { return m_sampler.getSampler(); }

private:
	Image m_image;
	Sampler m_sampler;
};

