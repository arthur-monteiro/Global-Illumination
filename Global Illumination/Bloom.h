#pragma once

#include "ComputePass.h"
#include "Blur.h"

class Bloom
{
public:
	Bloom() = default;
	~Bloom() = default;

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue computeQueue,
		Texture* inputTexture);
	void submit(VkDevice device, VkQueue computeQueue, std::vector<Semaphore*> semaphoresToWait);
	void cleanup(VkDevice device, VkCommandPool commandPool);

	// Getters
public:
	Semaphore* getSemaphore() { return &m_semaphoreMerge; }
	Texture* getOutputTexture() { return &m_outputTextureMerge; }
	bool isReady() { return m_isReady; }

private:
	bool m_isReady = false;
	
	ComputePass m_extractHighValues;
	Texture m_outputTextureExtract;
	Semaphore m_semaphoreExtractValues;
	
	Blur m_blur;
	
	ComputePass m_merge;
	Texture m_outputTextureMerge;
	Semaphore m_semaphoreMerge;
};

