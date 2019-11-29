#pragma once

#include <cstring>

#include "VulkanHelper.h"

struct UniformBufferObjectLayout
{
	VkShaderStageFlags accessibility;
	uint32_t binding;
};

class UniformBufferObject
{
public:
	UniformBufferObject() = default;
	~UniformBufferObject();

	bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, void* data, VkDeviceSize size);

	void updateData(VkDevice device, void* data);

	void cleanup(VkDevice device);

// Getters
public:
	VkBuffer getUniformBuffer() { return m_uniformBuffer; }
	VkDeviceSize getSize() { return m_size; }

private:
	VkBuffer m_uniformBuffer;
	VkDeviceMemory m_uniformBufferMemory;

	VkDeviceSize m_size;
};