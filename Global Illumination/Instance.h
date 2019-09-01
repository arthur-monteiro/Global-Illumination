#pragma once

#include "Vulkan.h"

class Instance
{
public:
	void load(Vulkan* vk, VkDeviceSize bufferSize, void* data);

	VkBuffer getInstanceBuffer() { return m_instanceBuffer; }
private:
	VkBuffer m_instanceBuffer;
	VkDeviceMemory m_instanceBufferMemory;
};

