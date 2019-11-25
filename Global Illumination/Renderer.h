#pragma once

#include "VulkanHelper.h"
#include "Pipeline.h"
#include "Model.h"
#include "UniformBufferObject.h"
#include "Texture.h"

class Renderer
{
public:
	Renderer() = default;
	~Renderer();

	void initialize(VkDevice device, std::string vertexShader, std::string fragmentShader, std::vector<VkVertexInputBindingDescription> vertexInputDescription, 
		std::vector<VkVertexInputAttributeDescription> attributeInputDescription, std::vector<UniformBufferObjectLayout> uniformBufferObjectLayouts, 
		std::vector<TextureLayout> textureLayouts, std::vector<bool> alphaBlending);
	void createPipeline(VkDevice device, VkRenderPass renderPass, VkExtent2D extent, VkSampleCountFlagBits msaa);

	int addModel(Model* model);

	void cleanup(VkDevice device);

	VkPipeline getPipeline() { return m_pipeline.getPipeline(); }
	std::vector<VertexBuffer> getVertexBuffers();

private:
	/* Information */
	std::string m_vertexShader;
	std::string m_fragmentShader;
	std::vector<VkVertexInputBindingDescription> m_vertexInputDescription;
	std::vector<VkVertexInputAttributeDescription> m_attributeInputDescription;
	std::vector<bool> m_alphaBlending;

	VkDescriptorSetLayout m_descriptorSetLayout;

	std::vector<Model*> m_models;

	Pipeline m_pipeline;

private:
	void createDescriptorSetLayout(VkDevice device, std::vector<UniformBufferObjectLayout> uniformBufferObjectLayouts, std::vector<TextureLayout> textureLayouts);
};

