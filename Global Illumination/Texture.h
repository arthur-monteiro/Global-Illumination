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

	bool createFromFile(VkDevice device, VkPhysicalDevice physicalDevice, std::string filename);

private:
	Image m_image;
	Sampler m_sampler;
};

