#include "Skybox.h"

void Skybox::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue graphicsQueue, VkExtent2D extent)
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
			{ glm::vec3(+0.5f, +0.5f, +0.5f), glm::vec2(1.0f, 1.0f) },
			{ glm::vec3(+0.5f, -0.5f, +0.5f), glm::vec2(1.0f, 0.0f) },
			{ glm::vec3(+0.5f, +0.5f, +0.5f), glm::vec2(1.0f, 1.0f) },
			{ glm::vec3(-0.5f, -0.5f, +0.5f), glm::vec2(0.0f, 0.0f) },
			{ glm::vec3(-0.5f, +0.5f, +0.5f), glm::vec2(0.0f, 1.0f) },

			{ glm::vec3(-0.5f, +0.5f, +0.5f), glm::vec2(1.0f, 0.0f) },
			{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f) },
			{ glm::vec3(-0.5f, +0.5f, -0.5f), glm::vec2(1.0f, 1.0f) },
			{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f) },
			{ glm::vec3(-0.5f, +0.5f, +0.5f), glm::vec2(1.0f, 0.0f) },
			{ glm::vec3(-0.5f, -0.5f, +0.5f), glm::vec2(0.0f, 0.0f) },

			{ glm::vec3(+0.5f, +0.5f, +0.5f), glm::vec2(1.0f, 0.0f) },
			{ glm::vec3(+0.5f, +0.5f, -0.5f), glm::vec2(1.0f, 1.0f) },
			{ glm::vec3(+0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f) },
			{ glm::vec3(+0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f) },
			{ glm::vec3(+0.5f, -0.5f, +0.5f), glm::vec2(0.0f, 0.0f) },
			{ glm::vec3(+0.5f, +0.5f, +0.5f), glm::vec2(1.0f, 0.0f) },

			{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f) },
			{ glm::vec3(+0.5f, -0.5f, +0.5f), glm::vec2(1.0f, 0.0f) },
			{ glm::vec3(+0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 1.0f) },	
			{ glm::vec3(+0.5f, -0.5f, +0.5f), glm::vec2(1.0f, 0.0f) },
			{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f) },
			{ glm::vec3(-0.5f, -0.5f, +0.5f), glm::vec2(0.0f, 0.0f) },

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

	std::array<Image, 6> tempImages;
	for (int i(0); i < 6; ++i)
		tempImages[i].createFromFile(device, physicalDevice, commandPool, graphicsQueue, facesTextures[i]);

	m_cubemap.createCubeMapFromImages(device, physicalDevice, commandPool, graphicsQueue, { &tempImages[0], &tempImages[1], &tempImages[2], &tempImages[3], &tempImages[4], &tempImages[5] } );

	for (int i(0); i < 6; ++i)
		tempImages[i].cleanup(device);

	// Render Pass
	m_attachments.resize(2);
	m_attachments[0].initialize(findDepthFormat(physicalDevice), VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, 
		VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	m_attachments[1].initialize(VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);

	m_renderPass.initialize(device, physicalDevice, commandPool, m_attachments, { extent });

	UniformBufferObjectLayout vpLayout;
	vpLayout.accessibility = VK_SHADER_STAGE_VERTEX_BIT;
	vpLayout.binding = 0;

	SamplerLayout samplerLayout;
	samplerLayout.accessibility = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerLayout.binding = 1;

	ImageLayout cubemapLayout;
	cubemapLayout.accessibility = VK_SHADER_STAGE_FRAGMENT_BIT;
	cubemapLayout.binding = 2;

	m_renderer.initialize(device, "Shaders/skybox/vert.spv", "Shaders/skybox/frag.spv", { Vertex3DTextured::getBindingDescription(0) }, Vertex3DTextured::getAttributeDescriptions(0),
		{ vpLayout }, {}, { cubemapLayout }, { samplerLayout }, {},  { false });

	glm::mat4 mat(1.0f);
	m_ubo.initialize(device, physicalDevice, &mat, sizeof(glm::mat4));

	m_sampler.initialize(device, VK_SAMPLER_ADDRESS_MODE_REPEAT, static_cast<float>(m_cubemap.getMipLevels()), VK_FILTER_LINEAR);

	m_renderer.addMesh(device, descriptorPool, m_mesh.getVertexBuffer(), { { &m_ubo, vpLayout} }, {}, { { &m_cubemap, cubemapLayout } }, { { &m_sampler, samplerLayout } }, {});

	m_clearValues.resize(2);
	m_clearValues[0] = { 1.0f };
	m_clearValues[1] = { 0.0f, 1.0f, 0.0f, 1.0f };

	m_renderPass.fillCommandBuffer(device, 0, m_clearValues, { &m_renderer }, VK_SAMPLE_COUNT_1_BIT);

	m_outputTexture.createFromImage(device, m_renderPass.getImages(0)[1]); // takes attachment 1 (0 is depth)
}

void Skybox::submit(VkDevice device, VkQueue graphicsQueue, glm::mat4 view, glm::mat4 projection)
{
	view = glm::mat4(glm::mat3(view));
	glm::mat4 vp = projection * view;
	m_ubo.updateData(device, &vp);

	m_renderPass.submit(device, graphicsQueue, 0, {});
}

void Skybox::cleanup(VkDevice device, VkCommandPool commandPool, VkDescriptorPool descriptorPool)
{
	m_mesh.cleanup(device);
	m_renderPass.cleanup(device, commandPool);
	m_renderer.cleanup(device, descriptorPool);
	m_cubemap.cleanup(device);
	m_ubo.cleanup(device);
}

void Skybox::resize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue graphicsQueue, VkExtent2D extent)
{
	cleanup(device, commandPool, descriptorPool);
	initialize(device, physicalDevice, commandPool, descriptorPool, graphicsQueue, extent);
}
