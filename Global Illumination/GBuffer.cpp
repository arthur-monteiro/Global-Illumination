#include "GBuffer.h"

GBuffer::~GBuffer()
{

}

bool GBuffer::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkExtent2D extent, ModelPBR* model,
	glm::mat4 view, glm::mat4 projection, bool useDepthAsStorage)
{
    /* Main Render Pass */
    // Attachments -> depth + world pos + albedo + normal + (rougness + metal + ao)
    m_attachments.resize(5);
	VkImageUsageFlags depthUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	if (useDepthAsStorage)
		depthUsage |= VK_IMAGE_USAGE_STORAGE_BIT;
	VkImageLayout depthFinalLayout;
	if (useDepthAsStorage)
		depthFinalLayout = VK_IMAGE_LAYOUT_GENERAL;
	else
		depthFinalLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
	VkAttachmentStoreOp depthStoreOp;
	if (useDepthAsStorage)
		depthStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	else
		depthStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    m_attachments[0].initialize(findDepthFormat(physicalDevice), m_sampleCount, depthFinalLayout, depthStoreOp, depthUsage);
	m_attachments[1].initialize(VK_FORMAT_R32G32B32A32_SFLOAT, m_sampleCount, VK_IMAGE_LAYOUT_GENERAL, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT );
    m_attachments[2].initialize(VK_FORMAT_R32G32B32A32_SFLOAT, m_sampleCount, VK_IMAGE_LAYOUT_GENERAL, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    m_attachments[3].initialize(VK_FORMAT_R32G32B32A32_SFLOAT, m_sampleCount, VK_IMAGE_LAYOUT_GENERAL, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    m_attachments[4].initialize(VK_FORMAT_R32G32B32A32_SFLOAT, m_sampleCount, VK_IMAGE_LAYOUT_GENERAL, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);

    m_renderPass.initialize(device, physicalDevice, commandPool, m_attachments, { extent });

	UniformBufferObjectLayout mvpLayout;
	mvpLayout.accessibility = VK_SHADER_STAGE_VERTEX_BIT;
	mvpLayout.binding = 0;

	MatricesUBO ubo;
	ubo.projection = projection;
	ubo.view = view;
	ubo.model = model->getTransformation();

	m_uboMVP.initialize(device, physicalDevice, &ubo, sizeof(MatricesUBO));

	std::vector<TextureLayout> textureLayouts(120);
	for (int i(0); i < textureLayouts.size(); ++i)
	{
		textureLayouts[i].accessibility = VK_SHADER_STAGE_FRAGMENT_BIT;
		textureLayouts[i].binding = i + 1;
	}

	m_renderer.initialize(device, "Shaders/gbuffer/vert.spv", "Shaders/gbuffer/frag.spv", { VertexPBR::getBindingDescription(0) }, VertexPBR::getAttributeDescriptions(0),
		{ mvpLayout }, textureLayouts, { false, false, false, false });

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
	m_clearValues[1] = { 1.0f, 0.0f, 0.0f, 1.0f };
	m_clearValues[2] = { 1.0f, 0.0f, 0.0f, 1.0f };
	m_clearValues[3] = { 1.0f, 0.0f, 0.0f, 1.0f };
	m_clearValues[4] = { 1.0f, 0.0f, 0.0f, 1.0f };

	m_renderPass.fillCommandBuffer(device, 0, m_clearValues, { &m_renderer }, m_sampleCount);

    return true;
}

bool GBuffer::submit(VkDevice device, VkQueue graphicsQueue, glm::mat4 view, glm::mat4 model, glm::mat4 projection)
{
    MatricesUBO ubo {};
    ubo.view = view;
    ubo.model = model;
	ubo.projection = projection;
    m_uboMVP.updateData(device, &ubo);

	m_renderPass.submit(device, graphicsQueue, 0, {});

    return true;
}

void GBuffer::resize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkExtent2D extent)
{
	recreate(device, physicalDevice, commandPool, extent, m_sampleCount);
}

void GBuffer::changeSampleCount(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkExtent2D extent,
	VkSampleCountFlagBits sampleCount)
{
	for(Attachment& attachment : m_attachments)
	{
		attachment.setSampleCount(sampleCount);
	}
	m_sampleCount = sampleCount;

	recreate(device, physicalDevice, commandPool, extent, sampleCount);
}

void GBuffer::useDepthAsStorage(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkExtent2D extent, bool useDepthAsStorage)
{
	VkImageUsageFlags depthUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	if (useDepthAsStorage)
		depthUsage |= VK_IMAGE_USAGE_STORAGE_BIT;
	VkImageLayout depthFinalLayout;
	if (useDepthAsStorage)
		depthFinalLayout = VK_IMAGE_LAYOUT_GENERAL;
	else
		depthFinalLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
	VkAttachmentStoreOp depthStoreOp;
	if (useDepthAsStorage)
		depthStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	else
		depthStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	// Initialize just change properties
	m_attachments[0].initialize(findDepthFormat(physicalDevice), m_sampleCount, depthFinalLayout, depthStoreOp, depthUsage);

	recreate(device, physicalDevice, commandPool, extent, m_sampleCount);
}

void GBuffer::cleanup(VkDevice device, VkCommandPool commandPool, VkDescriptorPool descriptorPool)
{
	m_renderPass.cleanup(device, commandPool);
	m_renderer.cleanup(device, descriptorPool);
}

void GBuffer::recreate(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
	VkExtent2D extent, VkSampleCountFlagBits sampleCount)
{
	m_renderPass.cleanup(device, commandPool);
	m_renderPass.initialize(device, physicalDevice, commandPool, m_attachments, { extent });

	m_renderer.setPipelineCreated(false);

	m_renderPass.fillCommandBuffer(device, 0, m_clearValues, { &m_renderer }, sampleCount);
}
