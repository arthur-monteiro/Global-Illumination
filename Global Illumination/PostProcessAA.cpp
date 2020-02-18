#include "PostProcessAA.h"

void PostProcessAA::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue computeQueue, Texture* inputTexture, Texture* inputDepth,
	Texture* inputDepthMS)
{
	m_outputTexture.create(device, physicalDevice, inputTexture->getImage()->getExtent(), VK_IMAGE_USAGE_STORAGE_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_outputTexture.setImageLayout(device, commandPool, computeQueue, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

	std::vector<std::pair<Texture*, TextureLayout>> texturesForComputePass;

	{
		TextureLayout textureLayout{};
		textureLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
		textureLayout.binding = 0;

		texturesForComputePass.emplace_back(inputTexture, textureLayout);
	}

	{
		TextureLayout textureLayout{};
		textureLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
		textureLayout.binding = 1;

		texturesForComputePass.emplace_back(inputDepth, textureLayout);
	}
	
	{
		TextureLayout textureLayout{};
		textureLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
		textureLayout.binding = 2;

		texturesForComputePass.emplace_back(inputDepthMS, textureLayout);
	}

	glm::int32 sampleCount = inputDepthMS->getImage()->getSampleCount();
	m_ubo.initialize(device, physicalDevice, &sampleCount, sizeof(glm::int32));

	UniformBufferObjectLayout uboLayout;
	uboLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	uboLayout.binding = 4;

	// Create deferred AA compute pass
	std::vector<std::pair<Texture*, TextureLayout>> textures = texturesForComputePass;

	TextureLayout textureLayout{};
	textureLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	textureLayout.binding = textures.size();

	textures.emplace_back(&m_outputTexture, textureLayout);

	std::string shaderPath = "Shaders/postProcess/deferredMSAA.spv";

	m_pass.initialize(device, physicalDevice, commandPool, descriptorPool, inputTexture->getImage()->getExtent(),
		{ 16, 16, 1 }, shaderPath, { { &m_ubo, uboLayout } },
		textures, {}, {});

	m_processFinishedSemaphore.initialize(device);
	m_processFinishedSemaphore.setPipelineStage(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}

void PostProcessAA::submit(VkDevice device, VkQueue computeQueue, std::vector<Semaphore*> semaphoresToWait)
{
	m_pass.submit(device, computeQueue, std::move(semaphoresToWait), m_processFinishedSemaphore.getSemaphore());
}

void PostProcessAA::cleanup(VkDevice device, VkCommandPool commandPool)
{
	m_pass.cleanup(device, commandPool);
	m_ubo.cleanup(device);
}
