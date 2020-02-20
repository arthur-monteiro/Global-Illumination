#pragma once

#include "ComputePass.h"

class SSR
{
public:
	SSR() = default;
	~SSR() = default;

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue computeQueue,
		Texture* inputShaded, Texture* inputViewPos, Texture* inputNormal, glm::mat4 projection);
	void submit(VkDevice device, VkQueue computeQueue, std::vector<Semaphore*> semaphoresToWait);
	void cleanup(VkDevice device, VkCommandPool commandPool);

	// Getters
public:
	Semaphore* getSemaphore() { return &m_semaphore; }
	Texture* getOutputTexture() { return &m_outputTexture; }

private:
	ComputePass m_pass;
	Texture m_outputTexture;
	Semaphore m_semaphore;
	UniformBufferObject m_ubo;
};

