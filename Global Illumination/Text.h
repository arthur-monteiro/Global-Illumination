#pragma once

#include <utility>


#include "VulkanHelper.h"
#include "Mesh.h"
#include "Font.h"
#include "UniformBufferObject.h"

class Text
{
public:
	Text() = default;
	~Text() = default;

	int addWString(std::wstring text, glm::vec2 position, glm::vec3 color);
	void build(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkExtent2D outputExtent, Font* font, float size);
	void cleanup(VkDevice device);

	float simulateSizeX(std::wstring text, VkExtent2D outputExtent, Font* font, float maxSize);

	VertexBuffer getVertexBuffer() { return m_mesh.getVertexBuffer(); }
	UniformBufferObject* getUBO() { return &m_ubo; }

private:
	struct TextStructure
	{
		glm::vec2 position;
		std::wstring textValue;
		glm::vec3 color;

		TextStructure(const glm::vec2 pos, std::wstring text, glm::vec3 color) : position(pos), textValue(std::move(text)), color(color) {}
	};
	std::vector<TextStructure> m_texts;
	
	Mesh<Vertex2DTexturedWithMaterial> m_mesh;
	UniformBufferObject m_ubo;
	std::vector<glm::vec4> m_uboData;
};

