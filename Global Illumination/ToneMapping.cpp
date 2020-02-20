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

	std::string shaderPath = "Shaders/postProcess/toneMapping.spv";

	m_pass.initialize(device, physicalDevice, commandPool, descriptorPool, inputTexture->getImage()->getExtent(),
		{ 16, 16, 1 }, shaderPath, { },
		{ { inputTexture, textureInputLayout}, { &m_outputTexture, textureOutputLayout} }, {}, {});
}

void ToneMapping::submit(VkDevice device, VkQueue computeQueue, std::vector<Semaphore*> semaphoresToWait)
{
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
