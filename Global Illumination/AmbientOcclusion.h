#pragma once

#include "SSAO.h"

class AmbientOcclusion
{
public:
	AmbientOcclusion() = default;
	~AmbientOcclusion() = default;

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue computeQueue, VkExtent2D extent);
	void submit(VkDevice device, VkQueue computeQueue, std::vector<Semaphore*> semaphoresToWait);
	void cleanup(VkDevice device, VkCommandPool commandPool);

	bool setAlgorithm(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue computeQueue, VkDescriptorPool descriptorPool, glm::mat4 projection,
		Texture* viewPosTexture, Texture* normalTexture, std::wstring algorithm);
	void recreate(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue computeQueue, VkDescriptorPool descriptorPool, glm::mat4 projection,
		Texture* viewPosTexture, Texture* normalTexture);
	void updateSSAOPower(VkDevice device, int power);

	// Getters
	Semaphore* getSemaphore();
	Texture* getTextureOutput();
	bool isReady() { return m_currentAlgorithm != L""; }

private:
	void createDefaultTexture(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue computeQueue, VkExtent2D extent);

private:
	std::wstring m_currentAlgorithm = L"";
	Texture m_defaultTexture;

	SSAO m_ssao;
};