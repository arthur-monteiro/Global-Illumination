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

	// Getters / setters
public:
	Semaphore* getSemaphore() { return &m_semaphore; }
	Texture* getOutputTexture() { return &m_outputTexture; }

	void setExposure(float exposure);
	void setGamma(float gamma);

private:
	ComputePass m_pass;
	Texture m_outputTexture;
	Semaphore m_semaphore;

	struct Params
	{
		glm::vec4 params = glm::vec4(1.0f, 2.2f, 0.0, 0.0);
	};
	Params m_params;
	UniformBufferObject m_ubo;

	bool m_needUpdateUBO = false;
};

