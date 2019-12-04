#include "GBuffer.h"

GBuffer::~GBuffer()
{

}

bool GBuffer::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkExtent2D extent, ModelPBR* model,
	glm::mat4 mvp)
{
    /* Main Render Pass */
    // Attachments -> depth + world pos + albedo + normal + (rougness + metal + ao)
    m_attachments.resize(5);
    m_attachments[0].initialize(findDepthFormat(physicalDevice), VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	m_attachments[1].initialize(VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    m_attachments[2].initialize(VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    m_attachments[3].initialize(VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    m_attachments[4].initialize(VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);

    m_renderPass.initialize(device, physicalDevice, surface, commandPool, m_attachments, { extent });

	UniformBufferObjectLayout mvpLayout;
	mvpLayout.accessibility = VK_SHADER_STAGE_VERTEX_BIT;
	mvpLayout.binding = 0;

	MVP_UBO ubo;
	ubo.mvp = mvp;
	ubo.model = model->getTransformation();

	m_uboMVP.initialize(device, physicalDevice, &ubo, sizeof(MVP_UBO));

	std::vector<TextureLayout> textureLayouts(5);
	for (int i(0); i < textureLayouts.size(); ++i)
	{
		textureLayouts[i].accessibility = VK_SHADER_STAGE_FRAGMENT_BIT;
		textureLayouts[i].binding = i + 1;
	}

	m_renderer.initialize(device, "Shaders/gbuffer/vert.spv", "Shaders/gbuffer/frag.spv", { VertexPBR::getBindingDescription(0) }, VertexPBR::getAttributeDescriptions(0),
		{ mvpLayout }, textureLayouts, { true, true, true, true });

	std::vector<VertexBuffer> vertexBuffers = model->getVertexBuffers();
	for (int i(0); i < vertexBuffers.size(); ++i)
	{
		std::vector<Texture*> textures = model->getTextures(i);
		std::vector<std::pair<Texture*, TextureLayout>> rendererTextures(textures.size());
		for (int j(0); j < rendererTextures.size(); ++j)
		{
			rendererTextures[j].first = textures[j];
			rendererTextures[j].second = textureLayouts[j];
		}

		m_renderer.addMesh(device, descriptorPool, vertexBuffers[i], { { &m_uboMVP, mvpLayout } }, rendererTextures);
	}

	m_clearValues.resize(5);
	m_clearValues[0] = { 1.0f };
	m_clearValues[1] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_clearValues[2] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_clearValues[3] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_clearValues[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	m_renderPass.fillCommandBuffer(device, 0, m_clearValues, { &m_renderer });

    return true;
}

bool GBuffer::submit(VkDevice device, VkQueue graphicsQueue, glm::mat4 mvp, glm::mat4 model)
{
    MVP_UBO ubo {};
    ubo.mvp = mvp;
    ubo.model = model;
    m_uboMVP.updateData(device, &ubo);

	m_renderPass.submit(device, graphicsQueue, 0, {});

    return true;
}

void GBuffer::resize(VkDevice device, VkPhysicalDevice physicalDevice, int width, int height)
{

}

void GBuffer::cleanup(VkDevice device, VkCommandPool commandPool, VkDescriptorPool descriptorPool)
{
	m_renderPass.cleanup(device, commandPool);
	m_renderer.cleanup(device, descriptorPool);
}
