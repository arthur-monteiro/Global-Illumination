#pragma once

#include "RenderPass.h"
#include "ModelPBR.h"

class GBuffer
{
public:
    GBuffer() = default;
    ~GBuffer();

    bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkExtent2D extent, ModelPBR* model,
		glm::mat4 mvp);
    bool submit(VkDevice device, VkQueue graphicsQueue, glm::mat4 mvp, glm::mat4 model);
    void resize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkExtent2D extent);
	void changeSampleCount(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkExtent2D extent, VkSampleCountFlagBits sampleCount);

    void cleanup(VkDevice device, VkCommandPool commandPool, VkDescriptorPool descriptorPool);

// Getters
public:
	std::vector<Image*> getImages() { return m_renderPass.getImages(0); }
	Semaphore * getRenderCompleteSemaphore() { return m_renderPass.getRenderCompleteSemaphore(); }

private:
	void recreate(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkExtent2D extent, VkSampleCountFlagBits sampleCount);
	
private:
	struct MatricesUBO
	{
		glm::mat4 mvp;
		glm::mat4 model;
	};

    RenderPass m_renderPass;
    std::vector<Attachment> m_attachments;
    std::vector<VkClearValue> m_clearValues;
	Renderer m_renderer;
	UniformBufferObject m_uboMVP;
	VkSampleCountFlagBits m_sampleCount = VK_SAMPLE_COUNT_1_BIT;
};