#include "Font.h"

void Font::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, int ySize, std::string path)
{
	m_maxYSize = ySize;

	unsigned int texWidth(0), texHeight(0), texChannels(0);
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
		throw std::runtime_error("Error : freeType init");

	FT_Face face;
	if (FT_New_Face(ft, path.c_str(), 0, &face))
		throw std::runtime_error("Error : font loading");

	FT_Set_Pixel_Sizes(face, 0, ySize);

	for (wchar_t c = 0; c < 255; ++c)
	{
		if (c == 160 || c == 32)
			c++;

		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			throw std::runtime_error(&"Error : character loading "[c]);

		if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) /* convert to an anti-aliased bitmap */
			throw std::runtime_error(&"Error : render glyph "[c]);

		texWidth = face->glyph->bitmap.width;
		texHeight = face->glyph->bitmap.rows;

		if (texWidth == 0 || texHeight == 0)
			throw std::runtime_error(&"Error : create pixel from character "[c]);

		auto* pixels = new unsigned char[static_cast<size_t>(texWidth) * texHeight];
		memcpy(pixels, face->glyph->bitmap.buffer, static_cast<size_t>(texWidth) * texHeight);

		m_characters[c].xSize = texWidth;
		m_characters[c].ySize = texHeight;
		m_characters[c].bearingX = face->glyph->bitmap_left;
		m_characters[c].bearingY = texHeight - face->glyph->bitmap_top;

		m_textures.emplace_back();
		m_textures[m_textures.size() - 1].createFromPixels(device, physicalDevice, commandPool, graphicsQueue, { texWidth, texHeight, 1 }, VK_FORMAT_R8_UNORM, pixels);
		m_textures[m_textures.size() - 1].createSampler(device, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 1.0f, VK_FILTER_LINEAR);

		m_characters[c].textureID = m_textures.size() - 1;

		delete[] pixels;
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}

void Font::cleanup(VkDevice device)
{
	for(Texture& texture : m_textures)
	{
		texture.cleanup(device);
	}
	m_textures.clear();
	m_characters.clear();
}

std::vector<Texture*> Font::getTextures()
{
	std::vector<Texture*> r(m_textures.size());

	for (int i(0); i < m_textures.size(); ++i)
		r[i] = &m_textures[i];

	return r;
}
