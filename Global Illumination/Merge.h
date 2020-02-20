#pragma once

#include "ComputePass.h"

class Merge
{
public:
	Merge() = default;
	~Merge() = default;

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool,
		Texture* inputTexture, Texture* inputHUD, std::vector<Texture*> swapChainTextures, std::vector<Operation> transitSwapChainToLayoutGeneral,
		std::vector<Operation> transitSwapChainToLayoutPresent);
	void submit(VkDevice device, VkQueue computeQueue, unsigned int swapChainImageIndex, std::vector<Semaphore*> semaphoresToWait);
	void cleanup(VkDevice device, VkCommandPool commandPool);

	void recreate(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool,
		Texture* inputTexture, Texture* inputHUD, std::vector<Texture*> swapChainTextures, std::vector<Operation> transitSwapChainToLayoutGeneral,
		std::vector<Operation> transitSwapChainToLayoutPresent);

	// Getters
public:
	Semaphore* getSemaphore() { return &m_semaphore; }

private:
	std::vector<ComputePass> m_passes;
	Semaphore m_semaphore;
};

