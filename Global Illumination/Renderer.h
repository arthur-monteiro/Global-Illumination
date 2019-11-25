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
	void createPipeline(VkDevice device, VkRenderPass renderPass);

	int addModel(Model* model);

private:
	/* Information */
	std::string m_vertexShader;
	std::string m_fragmentShader;

	std::vector<Model*> m_models;

	Pipeline m_pipeline;
};

