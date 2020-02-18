#include "Skybox.h"

void Skybox::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkExtent2D extent)
{
	// Ressources 
	std::vector<Vertex3DTextured> vertices = {
			{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f) },
			{ glm::vec3(+0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f) },
			{ glm::vec3(+0.5f, +0.5f, -0.5f), glm::vec2(1.0f, 1.0f) },
			{ glm::vec3(+0.5f, +0.5f, -0.5f), glm::vec2(1.0f, 1.0f) },
			{ glm::vec3(-0.5f, +0.5f, -0.5f), glm::vec2(0.0f, 1.0f) },
			{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f) },

			{ glm::vec3(-0.5f, -0.5f, +0.5f), glm::vec2(0.0f, 0.0f) },
			{ glm::vec3(+0.5f, -0.5f, +0.5f), glm::vec2(1.0f, 0.0f) },
			{ glm::vec3(+0.5f, +0.5f, +0.5f), glm::vec2(1.0f, 1.0f) },
			{ glm::vec3(+0.5f, +0.5f, +0.5f), glm::vec2(1.0f, 1.0f) },
			{ glm::vec3(-0.5f, +0.5f, +0.5f), glm::vec2(0.0f, 1.0f) },
			{ glm::vec3(-0.5f, -0.5f, +0.5f), glm::vec2(0.0f, 0.0f) },

			{ glm::vec3(-0.5f, +0.5f, +0.5f), glm::vec2(1.0f, 0.0f) },
			{ glm::vec3(-0.5f, +0.5f, -0.5f), glm::vec2(1.0f, 1.0f) },
			{ glm::vec3(-0.5f, +0.5f, -0.5f), glm::vec2(0.0f, 1.0f) },
			{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f) },
			{ glm::vec3(-0.5f, -0.5f, +0.5f), glm::vec2(0.0f, 0.0f) },
			{ glm::vec3(-0.5f, +0.5f, +0.5f), glm::vec2(1.0f, 0.0f) },

			{ glm::vec3(+0.5f, +0.5f, +0.5f), glm::vec2(1.0f, 0.0f) },
			{ glm::vec3(+0.5f, +0.5f, -0.5f), glm::vec2(1.0f, 1.0f) },
			{ glm::vec3(+0.5f, +0.5f, -0.5f), glm::vec2(0.0f, 1.0f) },
			{ glm::vec3(+0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f) },
			{ glm::vec3(+0.5f, -0.5f, +0.5f), glm::vec2(0.0f, 0.0f) },
			{ glm::vec3(+0.5f, +0.5f, +0.5f), glm::vec2(1.0f, 0.0f) },

			{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f) },
			{ glm::vec3(+0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 1.0f) },
			{ glm::vec3(+0.5f, -0.5f, +0.5f), glm::vec2(1.0f, 0.0f) },
			{ glm::vec3(+0.5f, -0.5f, +0.5f), glm::vec2(1.0f, 0.0f) },
			{ glm::vec3(-0.5f, -0.5f, +0.5f), glm::vec2(0.0f, 0.0f) },
			{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f) },

			{ glm::vec3(-0.5f, +0.5f, -0.5f), glm::vec2(0.0f, 1.0f) },
			{ glm::vec3(+0.5f, +0.5f, -0.5f), glm::vec2(1.0f, 1.0f) },
			{ glm::vec3(+0.5f, +0.5f, +0.5f), glm::vec2(1.0f, 0.0f) },
			{ glm::vec3(+0.5f, +0.5f, +0.5f), glm::vec2(1.0f, 0.0f) },
			{ glm::vec3(-0.5f, +0.5f, +0.5f), glm::vec2(0.0f, 0.0f) },
			{ glm::vec3(-0.5f, +0.5f, -0.5f), glm::vec2(0.0f, 1.0f) }
	};

	std::vector<uint32_t> indices;
	for (int i(0); i < vertices.size(); ++i)
		indices.push_back(i);

	m_mesh.loadFromVertices(device, physicalDevice, commandPool, graphicsQueue, vertices, indices);

	std::array<std::string, 6> facesTextures =
	{
		"Textures/skybox/right.jpg",
		"Textures/skybox/left.jpg",
		"Textures/skybox/top.jpg",
		"Textures/skybox/bottom.jpg",
		"Textures/skybox/front.jpg",
		"Textures/skybox/back.jpg"
	};

	for (int i(0); i < 6; ++i)
		m_textures[i].createFromFile(device, physicalDevice, commandPool, graphicsQueue, facesTextures[i]);

	// Render Pass
	m_attachments.resize(2);
	m_attachments[0].initialize(findDepthFormat(physicalDevice), VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, 
		VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	m_attachments[1].initialize(VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);

	m_renderPass.initialize(device, physicalDevice, commandPool, m_attachments, { extent });
}
