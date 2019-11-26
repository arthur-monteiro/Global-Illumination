#pragma once

#include <array>

#include "VulkanHelper.h"

class DescriptorPool
{
public:
	DescriptorPool() = default;
	~DescriptorPool();

	void initialize(VkDevice device);

	void cleanup(VkDevice device);

	VkDescriptorPool getDescriptorPool() { return m_descriptorPool; }

private:
	VkDescriptorPool m_descriptorPool;
};

