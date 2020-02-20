#pragma once

#include "ComputePass.h"

class Blur
{
public:
	Blur() = default;
	~Blur() = default;

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue computeQueue, Texture* inputTexture, int blurAmount);
	void submit(VkDevice device, VkQueue computeQueue, std::vector<Semaphore*> semaphoresToWait);
	void cleanup(VkDevice device, VkCommandPool commandPool);

	Semaphore* getSemaphore() { return &m_computePasses[m_computePasses.size() - 1].second; }

private:
	std::vector<std::pair<ComputePass, Semaphore>> m_computePasses;
	Texture m_texture;
};

