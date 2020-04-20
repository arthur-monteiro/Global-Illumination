#pragma once

#include "ComputePass.h"
#include <chrono>

class MotionBlur
{
public:
	MotionBlur() = default;
	~MotionBlur() = default;

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
		VkDescriptorPool descriptorPool, VkQueue computeQueue, Texture* viewPos, Texture* color);
	void submit(VkDevice device, VkQueue computeQueue, std::vector<Semaphore*> semaphoresToWait, glm::mat4 invView, glm::mat4 previousView, glm::mat4 projection);
	void cleanup(VkDevice device, VkCommandPool commandPool);

	Semaphore* getSemaphore() { return &m_semaphore; }
	Texture* getOutputTexture() { return &m_outputTexture; }
	
private:
	struct UBOMotionBlur
	{
		glm::mat4 invView;
		glm::mat4 previousView;
		glm::mat4 projection;
		float timeBetweenFrames;
	};
	UBOMotionBlur m_uboData;
	UniformBufferObject m_ubo;

	Texture m_velocityTexture;
	
	ComputePass m_computePass;
	Texture m_outputTexture;
	Semaphore m_semaphore;

	std::chrono::steady_clock::time_point m_previousTimePoint = std::chrono::steady_clock::now();
};

