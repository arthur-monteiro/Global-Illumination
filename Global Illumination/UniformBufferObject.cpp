#include "UniformBufferObject.h"

UniformBufferObject::~UniformBufferObject()
{
}

bool UniformBufferObject::initialize(VkDevice device, VkPhysicalDevice physicalDevice, void* data, VkDeviceSize size)
{
	createBuffer(device, physicalDevice, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffer, m_uniformBufferMemory);

	m_size = size;

	void* pData;
	vkMapMemory(device, m_uniformBufferMemory, 0, m_size, 0, &pData);
	memcpy(pData, data, m_size);
	vkUnmapMemory(device, m_uniformBufferMemory);

	return true;
}

void UniformBufferObject::updateData(VkDevice device, void* data)
{
	void* pData;
	vkMapMemory(device, m_uniformBufferMemory, 0, m_size, 0, &pData);
	memcpy(pData, data, m_size);
	vkUnmapMemory(device, m_uniformBufferMemory);
}

void UniformBufferObject::cleanup(VkDevice device)
{
	if (m_size <= 0)
		return;

	vkDestroyBuffer(device, m_uniformBuffer, nullptr);
	vkFreeMemory(device, m_uniformBufferMemory, nullptr);

	m_size = 0;
}
