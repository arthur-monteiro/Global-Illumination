#pragma once

#include "Vulkan.h"

class Sampler
{
public:
	void create(Vulkan* vk, VkSamplerAddressMode addressMode, float mipLevels, VkFilter filter);
	void cleanup(VkDevice device);

	VkSampler getSampler() { return m_textureSampler; }

private:
	VkSampler m_textureSampler = NULL;
};

