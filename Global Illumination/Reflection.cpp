#include "Reflection.h"

void Reflection::initialize()
{
}

void Reflection::submit(VkDevice device, VkQueue computeQueue, std::vector<Semaphore*> semaphoresToWait)
{
	m_ssr.submit(device, computeQueue, std::move(semaphoresToWait));
}

bool Reflection::setAlgorithm(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
                              VkDescriptorPool descriptorPool, VkQueue computeQueue, Texture* inputShaded, Texture* inputViewPos,
                              Texture* inputNormal, glm::mat4 projection, VkSampleCountFlagBits gBufferSampleCount, std::wstring algorithm)
{
	if (m_currentAlgorithm == algorithm)
		return false;

	if (algorithm == L"SSR")
	{
		m_ssr.initialize(device, physicalDevice, commandPool, descriptorPool, computeQueue, inputShaded, inputViewPos, inputNormal, projection, gBufferSampleCount);
	}
	else
		m_ssr.cleanup(device, commandPool);

	m_currentAlgorithm = algorithm;

	return true;
}

void Reflection::recreate(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
	VkDescriptorPool descriptorPool, VkQueue computeQueue, Texture* inputShaded, Texture* inputViewPos,
	Texture* inputNormal, glm::mat4 projection, VkSampleCountFlagBits gBufferSampleCount)
{
	if(m_currentAlgorithm == L"SSR")
	{
		m_ssr.cleanup(device, commandPool);
		m_ssr.initialize(device, physicalDevice, commandPool, descriptorPool, computeQueue, inputShaded, inputViewPos, inputNormal, projection, gBufferSampleCount);
	}
}

Semaphore* Reflection::getSemaphore()
{
	return m_ssr.getSemaphore();
}

Texture* Reflection::getTextureOutput()
{
	return m_ssr.getOutputTexture();
}
