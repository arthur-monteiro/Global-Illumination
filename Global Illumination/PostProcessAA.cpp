#include "PostProcessAA.h"

void PostProcessAA::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, Texture* inputTexture, Texture* inputDepth, 
	Texture* inputDepthMS, std::vector<Texture*> swapChainTextures, std::vector<Operation> transitSwapChainToLayoutGeneral, std::vector<Operation> transitSwapChainToLayoutPresent)
{
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

	// Create deferred AA compute pass
	m_deferredMsaaPasses.resize(swapChainTextures.size());
	for (size_t i(0); i < m_deferredMsaaPasses.size(); ++i)
	{
		std::vector<std::pair<Texture*, TextureLayout>> textures = texturesForComputePass;

		TextureLayout textureLayout{};
		textureLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
		textureLayout.binding = textures.size();

		textures.emplace_back(swapChainTextures[i], textureLayout);

		std::string shaderPath = "Shaders/postProcess/deferredMSAA.spv";

		m_deferredMsaaPasses[i].initialize(device, physicalDevice, commandPool, descriptorPool, swapChainTextures[0]->getImage()->getExtent(),
			{ 16, 16, 1 }, shaderPath, { },
			textures, { transitSwapChainToLayoutGeneral[i] }, { transitSwapChainToLayoutPresent[i] });
	}

	m_processFinishedSemaphore.initialize(device);
	m_processFinishedSemaphore.setPipelineStage(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}

void PostProcessAA::submit(VkDevice device, VkQueue computeQueue, unsigned swapChainImageIndex,
	std::vector<Semaphore*> semaphoresToWait)
{
	m_deferredMsaaPasses[swapChainImageIndex].submit(device, computeQueue, std::move(semaphoresToWait), m_processFinishedSemaphore.getSemaphore());
}

void PostProcessAA::cleanup(VkDevice device, VkCommandPool commandPool)
{
	for (size_t i(0); i < m_deferredMsaaPasses.size(); ++i)
	{
		m_deferredMsaaPasses[i].cleanup(device, commandPool);
	}
}
