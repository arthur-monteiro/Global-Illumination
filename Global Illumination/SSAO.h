#pragma once

#include "ComputePass.h"
#include "Blur.h"

class SSAO
{
public:
	SSAO() = default;
	~SSAO() = default;

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue computeQueue, VkDescriptorPool descriptorPool, glm::mat4 projection,
		Texture* viewPosTexture, Texture* normalTexture);
	void submit(VkDevice device, VkQueue computeQueue, std::vector<Semaphore*> semaphoresToWait);
	void cleanup(VkDevice device, VkCommandPool commandPool);

	void changePower(VkDevice device, int power);

	// Getters
	Semaphore* getSemaphore() { return m_blur.getSemaphore(); }
	Texture* getTextureOutput() { return &m_outputTexture; }

private:
	ComputePass m_computePass;
	Texture m_outputTexture;
	Semaphore m_renderFinishedSemaphore;

	Blur m_blur;

	struct SsaoUBO
	{
		glm::mat4 projection;
		glm::vec4 power = glm::vec4(1.0f);
		std::array<glm::vec4, 16> samples;
		std::array<glm::vec4, 16> noise;
	};
	SsaoUBO m_uboData;
	UniformBufferObject m_ubo;
};