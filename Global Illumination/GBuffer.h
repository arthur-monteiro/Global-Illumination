#pragma once

#include "RenderPass.h"
#include "ModelPBR.h"

class GBuffer
{
public:
    GBuffer() = default;
    ~GBuffer();

    bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkExtent2D extent, ModelPBR* model,
		glm::mat4 mvp);
    bool submit(VkDevice device, VkQueue graphicsQueue);
    void resize(VkDevice device, VkPhysicalDevice physicalDevice, int width, int height);

    void cleanup(VkDevice device, VkCommandPool commandPool, VkDescriptorPool descriptorPool);

// Getters
public:
	std::vector<Image*> getImages() { return m_renderPass.getImages(0); }
	Semaphore * getRenderCompleteSemaphore() { return m_renderPass.getRenderCompleteSemaphore(); }

private:
    RenderPass m_renderPass;
    std::vector<Attachment> m_attachments;
    std::vector<VkClearValue> m_clearValues;
	Renderer m_renderer;
	UniformBufferObject m_uboMVP;
};
