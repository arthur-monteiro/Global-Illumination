#include "Texture.h"

Texture::~Texture()
{
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

void Texture::cleanup(VkDevice device)
{
	m_image.cleanup(device);
	m_sampler.cleanup(device);
}
