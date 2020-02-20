#pragma once

#include "SSR.h"

class Reflection
{
public:
	Reflection() = default;
	~Reflection() = default;

	void initialize();
	void submit(VkDevice device, VkQueue computeQueue, std::vector<Semaphore*> semaphoresToWait);

	bool setAlgorithm(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue computeQueue,
		Texture* inputShaded, Texture* inputViewPos, Texture* inputNormal, glm::mat4 projection, std::wstring algorithm);
	void recreate(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue computeQueue,
		Texture* inputShaded, Texture* inputViewPos, Texture* inputNormal, glm::mat4 projection);

	Semaphore* getSemaphore();
	Texture* getTextureOutput();
	bool isReady() const { return !m_currentAlgorithm.empty() && m_currentAlgorithm != L"No"; }

private:
	std::wstring m_currentAlgorithm = L"";

	SSR m_ssr;
};

