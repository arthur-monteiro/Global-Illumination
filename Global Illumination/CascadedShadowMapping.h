#pragma once

#include "DepthPass.h"
#include "Blur.h"

class CascadedShadowMapping
{
public:
	CascadedShadowMapping() = default;
	~CascadedShadowMapping() = default;

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue graphicsQueue, VkExtent2D extent,
		ModelPBR* model, glm::vec3 sunDir, float cameraNear, float cameraFar);
	void submit(VkDevice device, VkQueue graphicsQueue, VkCommandPool commandPool, VkPhysicalDevice physicalDevice, VkDescriptorPool descriptorPool, glm::mat4 view, glm::mat4 model, glm::mat4 projection, float cameraNear, float cameraFOV, glm::vec3 lightDir,
		glm::vec3 cameraPosition, glm::vec3 cameraOrientation);
	void cleanup(VkDevice device, VkCommandPool commandPool, VkDescriptorPool descriptorPool);

	void setSoftShadowsOption(glm::uint softShadowsOption);
	void setSSIterations(glm::uint nIterations);
	void setSamplingDivisor(float divisor);
	void setBlurAmount(int blurAmount);

	// Getters
public:
	Semaphore* getSemaphore() { return m_blurAmount > 0 ? m_blur.getSemaphore() : m_renderPass.getRenderCompleteSemaphore(); }
	Texture* getOutputTexture() { return &m_outputTexture; }

private:
	void updateMatrices(float cameraNear, float cameraFOV, glm::vec3 lightDir, glm::vec3 cameraPosition, glm::vec3 cameraOrientation, glm::mat4 model);

private:
#define CASCADE_COUNT 4
	std::array<DepthPass, CASCADE_COUNT> m_depthPasses;

	RenderPass m_renderPass;
	std::vector<Attachment> m_attachments;
	std::vector<VkClearValue> m_clearValues;
	Renderer m_renderer;
	Texture m_outputTexture;

	Blur m_blur;
	int m_blurAmount = 0;
	int m_updateBlurAmount = -1;

	struct CascadedShadowMappingUBO
	{
		glm::mat4 mvp;
		std::array<glm::mat4, CASCADE_COUNT> lightSpaceMatrices;
		glm::vec4 cascadeSplits;
		glm::ivec4 softShadowsOption = glm::ivec4(0, 8, 700, 0);
	};
	CascadedShadowMappingUBO m_uboData;
	UniformBufferObject m_ubo;

	Sampler m_sampler;

	std::vector<float> m_cascadeSplits;
};

