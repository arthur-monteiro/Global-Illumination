#pragma once

#include "VulkanHelper.h"
#include "ComputePass.h"

class PostProcessAA
{
public:
	PostProcessAA() = default;
	~PostProcessAA() = default;

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue computeQueue, 
		Texture* inputTexture, Texture* inputDepth, Texture* inputDepthMS);
	void submit(VkDevice device, VkQueue computeQueue, std::vector<Semaphore*> semaphoresToWait);

	void cleanup(VkDevice device, VkCommandPool commandPool);

	// Getters
	Semaphore* getSemaphore() { return &m_processFinishedSemaphore; }
	Texture* getOutputTexture() { return &m_outputTexture; }
	
private:
	ComputePass m_pass;
	UniformBufferObject m_ubo;
	Texture m_outputTexture;

	Semaphore m_processFinishedSemaphore;
};