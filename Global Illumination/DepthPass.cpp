#include "DepthPass.h"

void DepthPass::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
	VkDescriptorPool descriptorPool, VkExtent2D extent, VkSampleCountFlagBits sampleCount, ModelPBR* model, glm::mat4 mvp)
{
	m_sampleCount = sampleCount;
	
	m_attachments.resize(1);
	m_attachments[0].initialize(findDepthFormat(physicalDevice), m_sampleCount, VK_IMAGE_LAYOUT_GENERAL, VK_ATTACHMENT_STORE_OP_STORE,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);

	m_renderPass.initialize(device, physicalDevice, commandPool, m_attachments, { extent });

	UniformBufferObjectLayout mvpLayout{};
	mvpLayout.accessibility = VK_SHADER_STAGE_VERTEX_BIT;
	mvpLayout.binding = 0;

	MatrixMVP_UBO ubo;
	ubo.mvp = mvp;

	m_uboMVP.initialize(device, physicalDevice, &ubo, sizeof(ubo));

	m_renderer.initialize(device, "Shaders/preDepthPass/vert.spv", { VertexPBR::getBindingDescription(0) }, VertexPBR::getAttributeDescriptions(0),
		{ mvpLayout }, {}, { false });

	std::vector<VertexBuffer> vertexBuffers = model->getVertexBuffers();
	for (int i(0); i < vertexBuffers.size(); ++i)
	{
		m_renderer.addMesh(device, descriptorPool, vertexBuffers[i], { { &m_uboMVP, mvpLayout } }, {}, {}, {});
	}

	m_clearValues.resize(1);
	m_clearValues[0] = { 1.0f };

	m_renderPass.fillCommandBuffer(device, 0, m_clearValues, { &m_renderer }, m_sampleCount);
}

void DepthPass::submit(VkDevice device, VkQueue graphicsQueue, glm::mat4 mvp)
{
	MatrixMVP_UBO ubo{};
	ubo.mvp = mvp;
	m_uboMVP.updateData(device, &ubo);

	m_renderPass.submit(device, graphicsQueue, 0, {});
}

void DepthPass::cleanup(VkDevice device, VkCommandPool commandPool, VkDescriptorPool descriptorPool)
{
	m_renderPass.cleanup(device, commandPool);
	m_renderer.cleanup(device, descriptorPool);
}
