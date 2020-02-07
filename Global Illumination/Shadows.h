#pragma once

#include "RayTracingShadows.h"

class Shadows
{
public:
	Shadows() = default;
	~Shadows() = default;

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, ModelPBR* model, glm::mat4 mvp, VkExtent2D extentOutput);
	void changeShadowType(const std::wstring& newType);
	void submit(VkDevice device, VkQueue queue, std::vector<Semaphore*> waitSemaphores, glm::mat4 viewInverse, glm::mat4 projInverse);

	void resize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, ModelPBR* model, VkExtent2D extentOutput);
	void changeRTSampleCount(VkDevice device, unsigned int sampleCount);

	void cleanup(VkDevice device, VkCommandPool commandPool);

	Image* getImage();
	Semaphore* getRenderCompleteSemaphore() { return &m_renderFinishedSemaphore; }
	

private:
	Semaphore m_renderFinishedSemaphore;

	enum class ShadowType { NO, RTX };
	ShadowType m_shadowType = ShadowType::NO;
	
	RayTracingShadows m_rtShadows;
	
};

