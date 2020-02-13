#pragma once

#include "VulkanHelper.h"

struct SamplerLayout
{
	VkShaderStageFlags accessibility;
	uint32_t binding;
};

class Sampler
{
public:
	Sampler() = default;
	~Sampler();

	void initialize(VkDevice device, VkSamplerAddressMode addressMode, float mipLevels, VkFilter filter);
	void cleanup(VkDevice device);

	VkSampler getSampler() { return m_textureSampler; }

private:
	VkSampler m_textureSampler = VK_NULL_HANDLE;
};

