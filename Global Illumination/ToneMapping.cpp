#include "ToneMapping.h"

void ToneMapping::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue computeQueue, Texture* inputTexture)
{
	m_outputTexture.create(device, physicalDevice, inputTexture->getImage()->getExtent(), VK_IMAGE_USAGE_STORAGE_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_outputTexture.setImageLayout(device, commandPool, computeQueue, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

	m_semaphore.initialize(device);
	m_semaphore.setPipelineStage(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

	TextureLayout textureInputLayout{};
	textureInputLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	textureInputLayout.binding = 0;

	TextureLayout textureOutputLayout{};
	textureOutputLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	textureOutputLayout.binding = 1;

	UniformBufferObjectLayout uboLayout;
	uboLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	uboLayout.binding = 2;

	m_ubo.initialize(device, physicalDevice, &m_params, sizeof(Params));

	std::string shaderPath = "Shaders/postProcess/toneMapping.spv";

	m_pass.initialize(device, physicalDevice, commandPool, descriptorPool, inputTexture->getImage()->getExtent(),
		{ 16, 16, 1 }, shaderPath, { { &m_ubo, uboLayout } },
		{ { inputTexture, textureInputLayout}, { &m_outputTexture, textureOutputLayout} }, {}, {});
}

void ToneMapping::submit(VkDevice device, VkQueue computeQueue, std::vector<Semaphore*> semaphoresToWait)
{
	if (m_needUpdateUBO)
	{
		m_ubo.updateData(device, &m_params);
		m_needUpdateUBO = false;
	}
	m_pass.submit(device, computeQueue, std::move(semaphoresToWait), m_semaphore.getSemaphore());
}

void ToneMapping::cleanup(VkDevice device, VkCommandPool commandPool)
{
	m_outputTexture.cleanup(device);
	m_pass.cleanup(device, commandPool);
	m_semaphore.cleanup(device);
}

void ToneMapping::recreate(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue computeQueue, Texture* inputTexture)
{
	cleanup(device, commandPool);
	initialize(device, physicalDevice, commandPool, descriptorPool, computeQueue, inputTexture);
}

void ToneMapping::setExposure(float exposure)
{
	m_params.params.x = exposure;
	m_needUpdateUBO = true;
}

void ToneMapping::setGamma(float gamma)
{
	m_params.params.y = gamma;
	m_needUpdateUBO = true;
}
