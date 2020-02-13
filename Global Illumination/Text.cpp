#include "Text.h"

int Text::addWString(std::wstring text, glm::vec2 position, glm::vec3 color)
{
	m_texts.emplace_back(position, text, color);

	return m_texts.size() - 1;
}

void Text::build(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkExtent2D outputExtent, Font* font, float size)
{
	std::vector<Vertex2DTexturedWithMaterial> vertices;
	int maxSizeY = font->getMaxSizeY();
	float scale = (outputExtent.height / static_cast<float>(maxSizeY)) * size * 2.0f;

	int textID = 0;
	for (const TextStructure& text : m_texts)
	{
		float offsetX = 0.0f;
		glm::vec2 offset = /*glm::vec3(-1.0f, -1.0f, 0.0f) +*/ text.position;

		for (const wchar_t& character : text.textValue)
		{
			if (character == ' ')
				offsetX += font->getXSize('a') * 0.5f;
			else
			{
				glm::vec2 topLeft = { offsetX, maxSizeY - font->getYSize(character) + font->getBearingY(character) };
				glm::vec2 topRight = { offsetX + font->getXSize(character), maxSizeY - font->getYSize(character) + font->getBearingY(character) };
				glm::vec2 botLeft = { offsetX, maxSizeY + font->getBearingY(character) };
				glm::vec2 botRight = { offsetX + font->getXSize(character), maxSizeY + font->getBearingY(character) };

				Vertex2DTexturedWithMaterial vertex;
				vertex.pos = scale * topLeft / glm::vec2(outputExtent.width, outputExtent.height) + offset;
				vertex.texCoord = glm::vec2(0.0f, 0.0f);
				vertex.IDs = glm::uvec3(font->getMaterialID(character), textID, 0);
				vertices.push_back(vertex);

				vertex.pos = scale * topRight / glm::vec2(outputExtent.width, outputExtent.height) + offset;
				vertex.texCoord = glm::vec2(1.0f, 0.0f);
				vertex.IDs = glm::uvec3(font->getMaterialID(character), textID, 0);
				vertices.push_back(vertex);

				vertex.pos = scale * botLeft / glm::vec2(outputExtent.width, outputExtent.height) + offset;
				vertex.texCoord = glm::vec2(0.0f, 1.0f);
				vertex.IDs = glm::uvec3(font->getMaterialID(character), textID, 0);
				vertices.push_back(vertex);

				vertex.pos = scale * botRight / glm::vec2(outputExtent.width, outputExtent.height) + offset;
				vertex.texCoord = glm::vec2(1.0f, 1.0f);
				vertex.IDs = glm::uvec3(font->getMaterialID(character), textID, 0);
				vertices.push_back(vertex);

				offsetX += font->getXSize(character);
				offsetX += font->getXSize('a') * 0.1f;
			}
		}

		textID++;
	}

	std::vector<uint32_t> indices;
	std::vector<uint32_t> indicesPattern = { 0, 2, 1, 1, 2, 3 };
	for (uint32_t i(0); i < vertices.size(); i += 4)
	{
		for (const uint32_t& indicePattern : indicesPattern)
			indices.push_back(indicePattern + i);
	}

	m_mesh.loadFromVertices(device, physicalDevice, commandPool, graphicsQueue, vertices, indices);

	for (int i(0); i < m_texts.size(); ++i)
	{
		m_uboData.color[i] = glm::vec4(m_texts[i].color, 0.0f);
		m_uboData.posOffset[i] = glm::vec4(0.0f);
	}
	m_ubo.initialize(device, physicalDevice, &m_uboData, sizeof(m_uboData));
}

void Text::cleanup(VkDevice device)
{
	m_mesh.cleanup(device);
	m_texts.clear();
}

float Text::simulateSizeX(std::wstring text, VkExtent2D outputExtent, Font* font, float size)
{
	float offsetX = 0.0f;
	int maxSizeY = font->getMaxSizeY();
	float scale = (outputExtent.height / static_cast<float>(maxSizeY)) * size * 2.0f;

	for (const wchar_t& character : text)
	{
		if (character == ' ')
			offsetX += font->getXSize('a') * 0.5f;
		else
		{
			offsetX += font->getXSize(character);
			offsetX += font->getXSize('a') * 0.1f;
		}
	}

	return scale * offsetX / outputExtent.width;
}

void Text::setColor(VkDevice device, unsigned ID, glm::vec3 color)
{
	m_texts[ID].color = color;
	updateUBO(device);
}

void Text::translate(VkDevice device, unsigned ID, glm::vec2 offset)
{
	m_texts[ID].posOffset += offset;
	updateUBO(device);
}

void Text::updateUBO(VkDevice device)
{
	for (int i(0); i < m_texts.size(); ++i)
	{
		m_uboData.color[i] = glm::vec4(m_texts[i].color, 0.0f);
		m_uboData.posOffset[i] = glm::vec4(m_texts[i].posOffset, 0.0f, 0.0f);
	}
	m_ubo.updateData(device, &m_uboData);
}
