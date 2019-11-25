#include "Renderer.h"

Renderer::~Renderer()
{
}

void Renderer::initialize(VkDevice device, std::string vertexShader, std::string fragmentShader, std::vector<VkVertexInputBindingDescription> vertexInputDescription,
	std::vector<VkVertexInputAttributeDescription> attributeInputDescription, std::vector<UniformBufferObjectLayout> uniformBufferObjectLayouts, 
	std::vector<TextureLayout> textureLayouts, std::vector<bool> alphaBlending)
{
	m_vertexShader = vertexShader;
	m_fragmentShader = fragmentShader;
}

void Renderer::createPipeline(VkDevice device, VkRenderPass renderPass)
{
}

int Renderer::addModel(Model* model)
{
	m_models.push_back(model);
	
	return m_models.size() - 1;
}
