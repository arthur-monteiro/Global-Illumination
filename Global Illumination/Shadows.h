#pragma once

#include "RayTracingShadows.h"
#include "CascadedShadowMapping.h"

class Shadows
{
public:
	Shadows() = default;
	~Shadows() = default;

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue computeQueue, VkExtent2D extent);
	bool changeShadowType(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue graphicsQueue, ModelPBR* model, glm::mat4 mvp, glm::vec3 sunDir,
		float cameraNear, float cameraFar, VkExtent2D extentOutput,
		const std::wstring& newType);
	void submit(VkDevice device, VkQueue graphicsQueue, VkCommandPool commandPool, VkPhysicalDevice physicalDevice, VkDescriptorPool descriptorPool, std::vector<Semaphore*> waitSemaphores, glm::mat4 viewInverse, glm::mat4 projInverse, glm::mat4 view, glm::mat4 model, glm::mat4 projection,
		float cameraNear, float cameraFOV, glm::vec3 lightDir, glm::vec3 cameraPosition, glm::vec3 cameraOrientation);

	void resize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue graphicsQueue, ModelPBR* model, VkExtent2D extentOutput,
		glm::vec3 lightDir, float cameraNear, float cameraFar);
	void changeRTSampleCount(VkDevice device, unsigned int sampleCount);
	void changeSoftShadowsOption(glm::uint softShadowOption);
	void changeSoftShadowsIteration(glm::uint softShadowsIterations);
	void changeSoftShadowsDivisor(float divisor);
	void changeBlurAmount(int blurAmount);

	void cleanup(VkDevice device, VkCommandPool commandPool);

	Texture* getTexture();
	Semaphore* getRenderCompleteSemaphore();
	bool isReady() { return m_shadowType != ShadowType::NO; }

private:
	void createDefaultTexture(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue computeQueue, VkExtent2D extent);

private:
	Texture m_defaultTexture;
	Semaphore m_renderFinishedSemaphore;

	enum class ShadowType { NO, RTX, CSM };
	ShadowType m_shadowType = ShadowType::NO;

	RayTracingShadows m_rtShadows;
	CascadedShadowMapping m_csm;
};

