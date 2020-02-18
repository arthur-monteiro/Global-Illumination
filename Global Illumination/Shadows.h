#pragma once

#include "RayTracingShadows.h"

class Shadows
{
public:
	Shadows() = default;
	~Shadows() = default;

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue computeQueue, VkExtent2D extent);
	bool changeShadowType(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, ModelPBR* model, glm::mat4 mvp, VkExtent2D extentOutput,
		const std::wstring& newType);
	void submit(VkDevice device, VkQueue queue, std::vector<Semaphore*> waitSemaphores, glm::mat4 viewInverse, glm::mat4 projInverse);

	void resize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, ModelPBR* model, VkExtent2D extentOutput);
	void changeRTSampleCount(VkDevice device, unsigned int sampleCount);

	void cleanup(VkDevice device, VkCommandPool commandPool);

	Texture* getTexture();
	Semaphore* getRenderCompleteSemaphore() { return &m_renderFinishedSemaphore; }
	bool isReady() { return m_shadowType != ShadowType::NO; }

private:
	void createDefaultTexture(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue computeQueue, VkExtent2D extent);

private:
	Texture m_defaultTexture;
	Semaphore m_renderFinishedSemaphore;

	enum class ShadowType { NO, RTX, CSM };
	ShadowType m_shadowType = ShadowType::NO;
	
	RayTracingShadows m_rtShadows;
	
};

