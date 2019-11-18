#pragma once

#include "VulkanHelper.h"

class Pipeline
{
public:
	Pipeline() = default;
	~Pipeline();

	void initialize(VkDevice device, VkRenderPass renderPass, std::string vertexShader, std::string fragmentShader);
};

