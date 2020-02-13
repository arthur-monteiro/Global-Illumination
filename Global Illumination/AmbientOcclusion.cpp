#include "AmbientOcclusion.h"

void AmbientOcclusion::initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue computeQueue, VkExtent2D extent)
{
	createDefaultTexture(device, physicalDevice, commandPool, computeQueue, extent);
}

void AmbientOcclusion::submit(VkDevice device, VkQueue computeQueue, std::vector<Semaphore*> semaphoresToWait)
{
	m_ssao.submit(device, computeQueue, std::move(semaphoresToWait));
}

void AmbientOcclusion::cleanup(VkDevice device, VkCommandPool commandPool)
{
	if (m_currentAlgorithm == L"")
		m_defaultTexture.cleanup(device);
	else if(m_currentAlgorithm == L"SSAO")
		m_ssao.cleanup(device, commandPool);
}

bool AmbientOcclusion::setAlgorithm(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue computeQueue, VkDescriptorPool descriptorPool, glm::mat4 projection,
	Texture* viewPosTexture, Texture* normalTexture, std::wstring algorithm)
{
	if (algorithm == m_currentAlgorithm)
		return false;

	if (algorithm == L"SSAO")
		m_ssao.initialize(device, physicalDevice, commandPool, computeQueue, descriptorPool, projection, viewPosTexture, normalTexture);
	else if (algorithm == L"" && m_currentAlgorithm == L"SSAO")
		m_ssao.cleanup(device, commandPool);

	m_currentAlgorithm = algorithm;
	return true;
}

void AmbientOcclusion::recreate(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue computeQueue, VkDescriptorPool descriptorPool, glm::mat4 projection, Texture* viewPosTexture, Texture* normalTexture)
{
	if (m_currentAlgorithm == L"")
	{
		if (m_defaultTexture.getImage()->getExtent().width != viewPosTexture->getImage()->getExtent().width ||
			m_defaultTexture.getImage()->getExtent().height != viewPosTexture->getImage()->getExtent().height)
		{
			m_defaultTexture.cleanup(device);
			createDefaultTexture(device, physicalDevice, commandPool, computeQueue, viewPosTexture->getImage()->getExtent());
		}
	}
	else if (m_currentAlgorithm == L"SSAO")
	{
		m_ssao.cleanup(device, commandPool);
		m_ssao.initialize(device, physicalDevice, commandPool, computeQueue, descriptorPool, projection, viewPosTexture, normalTexture);
	}
}

void AmbientOcclusion::updateSSAOPower(VkDevice device, int power)
{
	m_ssao.changePower(device, power);
}

Semaphore* AmbientOcclusion::getSemaphore()
{
	return m_ssao.getSemaphore();
}

Texture* AmbientOcclusion::getTextureOutput()
{
	if (m_currentAlgorithm == L"")
		return &m_defaultTexture;

	return m_ssao.getTextureOutput();
}

void AmbientOcclusion::createDefaultTexture(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue computeQueue, VkExtent2D extent)
{
	m_defaultTexture.create(device, physicalDevice, extent, VK_IMAGE_USAGE_STORAGE_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	m_defaultTexture.setImageLayout(device, commandPool, computeQueue, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}
