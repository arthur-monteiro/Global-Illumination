#pragma once

#include "RenderPass.h"

class Skybox
{
public:
	Skybox() = default;
	~Skybox() = default;

	void initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue graphicsQueue, VkExtent2D extent);
	void submit(VkDevice device, VkQueue graphicsQueue, glm::mat4 view, glm::mat4 projection);
	void cleanup(VkDevice device, VkCommandPool commandPool, VkDescriptorPool descriptorPool);

	void resize(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkQueue graphicsQueue, VkExtent2D extent);

	// Getters
public:
	Texture* getOutputTexture() { return &m_outputTexture; }
	Semaphore* getRenderCompleteSemaphore() { return m_renderPass.getRenderCompleteSemaphore(); }
	
private:
	Mesh<Vertex3DTextured> m_mesh;
	Image m_cubemap;
	Sampler m_sampler;

	RenderPass m_renderPass;
	std::vector<Attachment> m_attachments;
	std::vector<VkClearValue> m_clearValues;
	Texture m_outputTexture;

	Renderer m_renderer;
	UniformBufferObject m_ubo;
};

