#include "Texture.h"

Texture::~Texture()
{
}

bool Texture::createFromFile(VkDevice device, VkPhysicalDevice physicalDevice, std::string filename)
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = static_cast<uint64_t>(texWidth * texHeight * 4);
	uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	if (!pixels)
		throw std::runtime_error("Error : loading image " + filename);

	return true;
}
