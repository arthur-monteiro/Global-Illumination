#pragma once

#include "VulkanHelper.h"
#include "Pipeline.h"
#include "Model.h"

class Renderer
{
public:
	Renderer() = default;
	~Renderer();

	void initialize(VkDevice device, std::string vertexShader, std::string fragmentShader);
	void createPipeline(VkDevice device, VkRenderPass renderPass);

	int addModel(Model* model);

private:
	/* Information */
	std::string m_vertexShader;
	std::string m_fragmentShader;

	std::vector<Model*> m_models;

	Pipeline m_pipeline;
};

