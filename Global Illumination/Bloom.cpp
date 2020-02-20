#include "Bloom.h"

void Bloom::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
	VkDescriptorPool descriptorPool, VkQueue computeQueue, Texture* inputTexture)
{
	// Extract high values
	{
		m_outputTextureExtract.create(device, physicalDevice, inputTexture->getImage()->getExtent(), VK_IMAGE_USAGE_STORAGE_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
		m_outputTextureExtract.setImageLayout(device, commandPool, computeQueue, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

		TextureLayout textureInputLayout{};
		textureInputLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
		textureInputLayout.binding = 0;

		TextureLayout textureOutputLayout{};
		textureOutputLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
		textureOutputLayout.binding = 1;

		std::string shaderPath = "Shaders/bloom/extractHighValues.spv";

		m_extractHighValues.initialize(device, physicalDevice, commandPool, descriptorPool, inputTexture->getImage()->getExtent(),
			{ 16, 16, 1 }, shaderPath, { },
			{ { inputTexture, textureInputLayout}, { &m_outputTextureExtract, textureOutputLayout} }, {}, {});

		m_semaphoreExtractValues.initialize(device);
		m_semaphoreExtractValues.setPipelineStage(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	}

	// Blur
	m_blur.initialize(device, physicalDevice, commandPool, descriptorPool, computeQueue, &m_outputTextureExtract, 4);

	// Merge
	{
		m_outputTextureMerge.create(device, physicalDevice, inputTexture->getImage()->getExtent(), VK_IMAGE_USAGE_STORAGE_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
		m_outputTextureMerge.setImageLayout(device, commandPool, computeQueue, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

		TextureLayout textureInputLayout{};
		textureInputLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
		textureInputLayout.binding = 0;

		TextureLayout textureInputBlurLayout{};
		textureInputBlurLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
		textureInputBlurLayout.binding = 1;

		TextureLayout textureOutputLayout{};
		textureOutputLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
		textureOutputLayout.binding = 2;

		std::string shaderPath = "Shaders/bloom/merge.spv";

		m_merge.initialize(device, physicalDevice, commandPool, descriptorPool, inputTexture->getImage()->getExtent(),
			{ 16, 16, 1 }, shaderPath, { },
			{ { inputTexture, textureInputLayout}, { &m_outputTextureExtract, textureInputBlurLayout}, { &m_outputTextureMerge, textureOutputLayout} }, {}, {});

		m_semaphoreMerge.initialize(device);
		m_semaphoreMerge.setPipelineStage(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	}

	m_isReady = true;
}

void Bloom::submit(VkDevice device, VkQueue computeQueue, std::vector<Semaphore*> semaphoresToWait)
{
	m_extractHighValues.submit(device, computeQueue, std::move(semaphoresToWait), m_semaphoreExtractValues.getSemaphore());
	m_blur.submit(device, computeQueue, { &m_semaphoreExtractValues });
	m_merge.submit(device, computeQueue, { m_blur.getSemaphore() }, m_semaphoreMerge.getSemaphore());
}

void Bloom::cleanup(VkDevice device, VkCommandPool commandPool)
{
	m_outputTextureExtract.cleanup(device);
	m_extractHighValues.cleanup(device, commandPool);
	m_semaphoreExtractValues.cleanup(device);

	m_isReady = false;
}
