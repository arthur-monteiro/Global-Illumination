#pragma once

#include "VulkanHelper.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

struct InstanceBuffer
{
	VkBuffer instanceBuffer;
	size_t nInstances;
};

struct InstanceSingleID
{
	glm::uint id;

	static VkVertexInputBindingDescription getBindingDescription(uint32_t binding)
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = binding;
		bindingDescription.stride = sizeof(InstanceSingleID);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

		return bindingDescription;
	}

	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(uint32_t binding, uint32_t startLocation)
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);

		attributeDescriptions[0].binding = binding;
		attributeDescriptions[0].location = startLocation;
		attributeDescriptions[0].format = VK_FORMAT_R32_UINT;
		attributeDescriptions[0].offset = 0;

		return attributeDescriptions;
	}
};

template <typename T>
class Instance
{
public:
	Instance() = default;
	~Instance() = default;

	void loadFromVector(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::vector<T> data);

public: // Getters
	InstanceBuffer getInstanceBuffer() const { return { m_instanceBuffer, m_instances.size() }; }

private:
	VkBuffer m_instanceBuffer				= nullptr;
	VkDeviceMemory m_instanceBufferMemory	= nullptr;

	std::vector<T> m_instances;
};

template <typename T>
void Instance<T>::loadFromVector(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::vector<T> data)
{
	m_instances = std::move(data);
	const VkDeviceSize bufferSize = sizeof(m_instances[0]) * m_instances.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* pData;
	vkMapMemory(device,  stagingBufferMemory, 0, bufferSize, 0, &pData);
		memcpy(pData, m_instances.data(), bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_instanceBuffer, m_instanceBufferMemory);

	copyBuffer(device, commandPool, graphicsQueue, stagingBuffer, m_instanceBuffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

