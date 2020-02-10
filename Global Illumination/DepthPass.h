#pragma once

#include "VulkanHelper.h"
#include "RenderPass.h"
#include "ModelPBR.h"

class DepthPass
{
public:
	DepthPass() = default;
	~DepthPass() = default;

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkExtent2D extent, VkSampleCountFlagBits sampleCount,
		ModelPBR* model, glm::mat4 mvp);
	void submit(VkDevice device, VkQueue graphicsQueue, glm::mat4 mvp);

	void cleanup(VkDevice device, VkCommandPool commandPool, VkDescriptorPool descriptorPool);

	// Getters
	Semaphore* getSemaphore() { return m_renderPass.getRenderCompleteSemaphore(); }
	Image* getImage() { return m_renderPass.getImages(0)[0]; }

private:
	struct MatrixMVP_UBO
	{
		glm::mat4 mvp;
	};
	
	RenderPass m_renderPass;
	std::vector<Attachment> m_attachments;
	std::vector<VkClearValue> m_clearValues;

	UniformBufferObject m_uboMVP;
	Renderer m_renderer;

	VkSampleCountFlagBits m_sampleCount = VK_SAMPLE_COUNT_1_BIT;
};

