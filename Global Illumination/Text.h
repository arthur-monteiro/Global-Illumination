#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H  

#include <map>

#include "Vulkan.h"
#include "Pipeline.h"
#include "UniformBufferObject.h"

struct Character
{
	VkImage image;
	VkDeviceMemory imageMemory;
	VkImageView imageView;
	int xSize; // en pixel
	int ySize;
	int bearingX;
	int bearingY;
};

class Text
{
public:
	void initialize(Vulkan * vk, int ySize, std::string path);
	int addText(Vulkan * vk, std::wstring text, glm::vec2 pos, float maxSize, glm::vec3 color);

	void changeText(Vulkan * vk, std::wstring text, int textID);

	float simulateSizeX(Vulkan* vk, std::wstring text, float maxSize);

public:
	int getNbTexts() { return (int)m_texts.size(); }
	int getNbCharacters(int index) { return (int)m_texts[index].character.size(); }
	VkBuffer getVertexBuffer(int indexI, int indexJ) { return m_texts[indexI].vertexBuffers[indexJ]; }
	UboBase* getUbo(int index) { return &m_texts[index].ubo; }
	VkBuffer getIndexBuffer() { return m_indexBuffer; }
	VkImageView getImageView(int indexI, int indexJ) { return m_characters[m_texts[indexI].character[indexJ]].imageView; }
	VkSampler getSampler() { return m_sampler; }

	void setColor(Vulkan* vk, int index, glm::vec3 color);

	int needUpdate() { return m_needUpdate; }
	void updateDone() { m_needUpdate = -1; }

private:
	struct TextStruct
	{
		glm::vec2 pos;
		float maxSize;
		std::vector<VkBuffer> vertexBuffers;
		std::vector<VkDeviceMemory> vertexBufferMemories;
		std::vector<wchar_t> character;

		glm::vec3 color;
		UniformBufferObject<UniformBufferObjectSingleVec> ubo;
	};

	TextStruct createTextStruct(Vulkan * vk, std::wstring text, glm::vec2 pos, float maxSize, glm::vec3 color);

private:
	std::map<wchar_t, Character> m_characters;
	VkSampler m_sampler;
	int m_maxYSize;

	std::vector<TextStruct> m_texts;
	int m_needUpdate = -1;

	VkBuffer m_indexBuffer;
	VkDeviceMemory m_indexBufferMemory;
};

