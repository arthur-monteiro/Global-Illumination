#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <map>

#include "VulkanHelper.h"
#include "Texture.h"

class Font
{
public:
	Font() = default;
	~Font() = default;

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, int ySize, std::string path);
	void cleanup(VkDevice device);

	unsigned int getXSize(const wchar_t character) { return m_characters[character].xSize; }
	unsigned int getYSize(const wchar_t character) { return m_characters[character].ySize; }
	unsigned int getBearingX(const wchar_t character) { return m_characters[character].bearingX; }
	unsigned int getBearingY(const wchar_t character) { return m_characters[character].bearingY; }
	int getMaxSizeY() const { return m_maxYSize; }
	unsigned int getMaterialID(const wchar_t character) { return m_characters[character].textureID; }
	std::vector<Image*> getImages();
	Sampler* getSampler() { return &m_sampler; }

private:
	struct Character
	{
		unsigned int xSize = 0; // en pixel
		unsigned int ySize = 0;
		unsigned int bearingX = 0;
		unsigned int bearingY = 0;

		unsigned int textureID = 0;;
	};

	std::map<wchar_t, Character> m_characters;
	std::vector<Image> m_images;
	Sampler m_sampler;
	int m_maxYSize;
};