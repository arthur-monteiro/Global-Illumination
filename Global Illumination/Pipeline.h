#pragma once

#include "VulkanHelper.h"

#include <fstream>

class Pipeline
{
public:
	Pipeline() = default;
	~Pipeline();

	void initialize(VkDevice device, VkRenderPass renderPass, std::string vertexShader, std::string fragmentShader, std::vector<VkVertexInputBindingDescription> vertexInputDescription,
		std::vector<VkVertexInputAttributeDescription> attributeInputDescription, VkExtent2D extent, VkSampleCountFlagBits msaaSamples, std::vector<bool> alphaBlending,
		VkDescriptorSetLayout* descriptorSetLayout);

	void cleanup(VkDevice device);

	VkPipeline getPipeline() { return m_pipeline; }

private:
	VkPipelineLayout m_pipelineLayout;
	VkPipeline m_pipeline;

private:
	static std::vector<char> readFile(const std::string& filename);
	static VkShaderModule createShaderModule(const std::vector<char>& code, VkDevice device);
};

