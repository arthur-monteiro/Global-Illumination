#include "Renderer.h"

Renderer::~Renderer()
{
}

void Renderer::initialize(VkDevice device, std::string vertexShader, std::string fragmentShader)
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
