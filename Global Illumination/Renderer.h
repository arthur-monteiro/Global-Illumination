#pragma once

#include "VulkanHelper.h"
#include "Pipeline.h"
#include "Model.h"
#include "UniformBufferObject.h"
#include "Texture.h"
#include "Instance.h"

class Renderer
{
public:
	Renderer() = default;
	~Renderer();

	void initialize(VkDevice device, std::string vertexShader, std::string fragmentShader, std::vector<VkVertexInputBindingDescription> vertexInputDescription, 
		std::vector<VkVertexInputAttributeDescription> attributeInputDescription, std::vector<UniformBufferObjectLayout> uniformBufferObjectLayouts, 
		std::vector<TextureLayout> textureLayouts, std::vector<bool> alphaBlending);
	void createPipeline(VkDevice device, VkRenderPass renderPass, VkExtent2D extent, VkSampleCountFlagBits msaa);
	void destroyPipeline(VkDevice device);

	int addMesh(VkDevice device, VkDescriptorPool descriptorPool, VertexBuffer vertexBuffer, 
		std::vector<std::pair<UniformBufferObject*, UniformBufferObjectLayout>> ubos, std::vector<std::pair<Texture*, TextureLayout>> textures);
	int addMeshInstancied(VkDevice device, VkDescriptorPool descriptorPool, VertexBuffer vertexBuffer, InstanceBuffer instanceBuffer,
		std::vector<std::pair<UniformBufferObject*, UniformBufferObjectLayout>> ubos, std::vector<std::pair<Texture*, TextureLayout>> textures);
	void reloadMeshVertexBuffer(VkDevice device, VertexBuffer vertexBuffer, int meshID);

	void cleanup(VkDevice device, VkDescriptorPool descriptorPool);

	VkPipeline getPipeline() { return m_pipeline.getPipeline(); }
	std::vector<std::pair<VertexBuffer, VkDescriptorSet>> getMeshes();
	std::vector<std::tuple<VertexBuffer, InstanceBuffer, VkDescriptorSet>> getMeshesInstancied();
	VkPipelineLayout getPipelineLayout() { return m_pipeline.getPipelineLayout(); }

	void setPipelineCreated(bool status) { m_pipelineCreated = status; }

private:
	/* Information */
	std::string m_vertexShader;
	std::string m_fragmentShader;
	std::vector<VkVertexInputBindingDescription> m_vertexInputDescription;
	std::vector<VkVertexInputAttributeDescription> m_attributeInputDescription;
	std::vector<bool> m_alphaBlending;

	VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;

	std::vector<std::pair<VertexBuffer, VkDescriptorSet>> m_meshes;
	std::vector<std::tuple<VertexBuffer, InstanceBuffer, VkDescriptorSet>> m_meshesInstancied;
	
	Pipeline m_pipeline;
	bool m_pipelineCreated = false;

private:
	void createDescriptorSetLayout(VkDevice device, std::vector<UniformBufferObjectLayout> uniformBufferObjectLayouts, std::vector<TextureLayout> textureLayouts);
	VkDescriptorSet createDescriptorSet(VkDevice device, VkDescriptorPool descriptorPool,
		std::vector<std::pair<UniformBufferObject*, UniformBufferObjectLayout>> ubos, std::vector<std::pair<Texture*, TextureLayout>> textures);
};

