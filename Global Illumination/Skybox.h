#pragma once

#include "RenderPass.h"

class Skybox
{
public:
	Skybox() = default;
	~Skybox() = default;

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkExtent2D extent);

private:
	Mesh<Vertex3DTextured> m_mesh;
	std::array<Texture, 6> m_textures;

	RenderPass m_renderPass;
	std::vector<Attachment> m_attachments;
	std::vector<VkClearValue> m_clearValues;
};

