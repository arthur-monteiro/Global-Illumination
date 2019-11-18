#pragma once

#include "VulkanHelper.h"

#include <cstring>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

struct Vertex2D
{
	glm::vec2 pos;

	static VkVertexInputBindingDescription getBindingDescription(uint32_t binding)
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = binding;
		bindingDescription.stride = sizeof(Vertex2D);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(uint32_t binding)
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);

		attributeDescriptions[0].binding = binding;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex2D, pos);

		return attributeDescriptions;
	}

	bool operator==(const Vertex2D& other) const
	{
		return pos == other.pos;
	}
};

template <typename T>
class Mesh
{
public:
	Mesh() = default;
	~Mesh() {}

	void loadFromVertices(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::vector<T> vertices, std::vector<uint32_t> indices)
	{
		m_vertices = vertices;
		m_indices = indices;

		createVertexBuffer(device, physicalDevice, commandPool, graphicsQueue, sizeof(m_vertices[0]) * m_vertices.size(), m_vertices.data());
		createIndexBuffer(device, physicalDevice, commandPool, graphicsQueue);
	}

private:
    // Vertex
	std::vector<T> m_vertices;
    VkBuffer m_vertexBuffer;
    VkDeviceMemory m_vertexBufferMemory;

    // Indices
    std::vector<uint32_t> m_indices;
    VkBuffer m_indexBuffer;
    VkDeviceMemory m_indexBufferMemory;

private:
	void createVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkDeviceSize size, void* data)
	{
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(device, physicalDevice, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* tData;
		vkMapMemory(device, stagingBufferMemory, 0, size, 0, &tData);
		std::memcpy(tData, data, (size_t)size);
		vkUnmapMemory(device, stagingBufferMemory);

		createBuffer(device, physicalDevice, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffer, m_vertexBufferMemory);

		copyBuffer(device, commandPool, graphicsQueue, stagingBuffer, m_vertexBuffer, size);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	void createIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
	{
		VkDeviceSize bufferSize = sizeof(m_indices[0]) * m_indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, m_indices.data(), (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBuffer, m_indexBufferMemory);

		copyBuffer(device, commandPool, graphicsQueue, stagingBuffer, m_indexBuffer, bufferSize);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}
};