#include "Merge.h"

void Merge::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, Texture* inputTexture, Texture* inputHUD,
	std::vector<Texture*> swapChainTextures, std::vector<Operation> transitSwapChainToLayoutGeneral, std::vector<Operation> transitSwapChainToLayoutPresent)
{
	m_semaphore.initialize(device);
	m_semaphore.setPipelineStage(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

	TextureLayout textureInputLayout{};
	textureInputLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	textureInputLayout.binding = 0;

	TextureLayout textureInputHUDLayout{};
	textureInputHUDLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	textureInputHUDLayout.binding = 1;

	TextureLayout textureOutputLayout{};
	textureOutputLayout.accessibility = VK_SHADER_STAGE_COMPUTE_BIT;
	textureOutputLayout.binding = 2;

	std::string shaderPath = "Shaders/postProcess/merge.spv";

	m_passes.resize(swapChainTextures.size());
	for (size_t i(0); i < m_passes.size(); ++i)
	{
		m_passes[i].initialize(device, physicalDevice, commandPool, descriptorPool, inputTexture->getImage()->getExtent(),
			{ 16, 16, 1 }, shaderPath, { },
			{ { inputTexture, textureInputLayout}, { inputHUD, textureInputHUDLayout}, { swapChainTextures[i], textureOutputLayout} },
			{ transitSwapChainToLayoutGeneral[i] }, { transitSwapChainToLayoutPresent[i] });
	}
}

void Merge::submit(VkDevice device, VkQueue computeQueue, unsigned int swapChainImageIndex, std::vector<Semaphore*> semaphoresToWait)
{
	m_passes[swapChainImageIndex].submit(device, computeQueue, std::move(semaphoresToWait), m_semaphore.getSemaphore());
}

void Merge::cleanup(VkDevice device, VkCommandPool commandPool)
{
	for (int i(0); i < m_passes.size(); ++i)
		m_passes[i].cleanup(device, commandPool);
	m_semaphore.cleanup(device);
}

void Merge::recreate(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, Texture* inputTexture, Texture* inputHUD, 
	std::vector<Texture*> swapChainTextures, std::vector<Operation> transitSwapChainToLayoutGeneral, std::vector<Operation> transitSwapChainToLayoutPresent)
{
	cleanup(device, commandPool);
	initialize(device, physicalDevice, commandPool, descriptorPool, inputTexture, inputHUD, std::move(swapChainTextures), std::move(transitSwapChainToLayoutGeneral), std::move(transitSwapChainToLayoutPresent));
}
