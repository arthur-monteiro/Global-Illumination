#pragma once

#include "ComputePass.h"

class ToneMapping
{
public:
	ToneMapping() = default;
	~ToneMapping() = default;

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue computeQueue,
		Texture* inputTexture);
	void submit(VkDevice device, VkQueue computeQueue, std::vector<Semaphore*> semaphoresToWait);
	void cleanup(VkDevice device, VkCommandPool commandPool);

	void recreate(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue computeQueue,
		Texture* inputTexture);

	// Getters
public:
	Semaphore* getSemaphore() { return &m_semaphore; }
	Texture* getOutputTexture() { return &m_outputTexture; }

private:
	ComputePass m_pass;
	Texture m_outputTexture;
	Semaphore m_semaphore;
};

