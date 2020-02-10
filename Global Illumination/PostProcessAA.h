#pragma once

#include "VulkanHelper.h"
#include "ComputePass.h"

class PostProcessAA
{
public:
	PostProcessAA() = default;
	~PostProcessAA() = default;

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, Texture* inputTexture, Texture* inputDepth, Texture* inputDepthMS,
		std::vector<Texture*> swapChainTextures, std::vector<Operation> transitSwapChainToLayoutGeneral, std::vector<Operation> transitSwapChainToLayoutPresent);
	void submit(VkDevice device, VkQueue computeQueue, unsigned int swapChainImageIndex, std::vector<Semaphore*> semaphoresToWait);

	void cleanup(VkDevice device, VkCommandPool commandPool);

	// Getters
	VkSemaphore getSemaphore() { return m_processFinishedSemaphore.getSemaphore(); }
	
private:
	std::vector<ComputePass> m_deferredMsaaPasses;

	Semaphore m_processFinishedSemaphore;
};