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

struct Vertex2DTextured
{
	glm::vec2 pos;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription(uint32_t binding)
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = binding;
		bindingDescription.stride = sizeof(Vertex2DTextured);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(uint32_t binding)
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

		attributeDescriptions[0].binding = binding;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex2DTextured, pos);

		attributeDescriptions[1].binding = binding;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex2DTextured, texCoord);

		return attributeDescriptions;
	}

	bool operator==(const Vertex2DTextured& other) const
	{
		return pos == other.pos && texCoord == other.texCoord;
	}
};

struct VertexPBR
{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec2 texCoord;
	glm::uint materialID;

    static VkVertexInputBindingDescription getBindingDescription(uint32_t binding)
    {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = binding;
        bindingDescription.stride = sizeof(VertexPBR);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(uint32_t binding)
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(5);

        attributeDescriptions[0].binding = binding;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(VertexPBR, pos);

        attributeDescriptions[1].binding = binding;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(VertexPBR, normal);

        attributeDescriptions[2].binding = binding;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(VertexPBR, tangent);

        attributeDescriptions[3].binding = binding;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(VertexPBR, texCoord);

		attributeDescriptions[4].binding = binding;
		attributeDescriptions[4].location = 4;
		attributeDescriptions[4].format = VK_FORMAT_R32_UINT;
		attributeDescriptions[4].offset = offsetof(VertexPBR, materialID);

        return attributeDescriptions;
    }

    bool operator==(const VertexPBR& other) const
    {
        return pos == other.pos && normal == other.normal && texCoord == other.texCoord && tangent == other.tangent;
    }
};

namespace std
{
    template<> struct hash<VertexPBR>
    {
        size_t operator()(VertexPBR const& vertex) const
        {
            return ((hash<glm::vec3>()(vertex.pos) ^
                     (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
                   (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

struct VertexBuffer
{
	VkBuffer vertexBuffer;
	unsigned int nbVertices;
	VkBuffer indexBuffer;
	unsigned int nbIndices;
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

	void cleanup(VkDevice device)
	{
		m_vertices.clear();
		m_indices.clear();

		vkDestroyBuffer(device, m_vertexBuffer, nullptr);
		vkFreeMemory(device, m_vertexBufferMemory, nullptr);

		vkDestroyBuffer(device, m_indexBuffer, nullptr);
		vkFreeMemory(device, m_indexBufferMemory, nullptr);
	}

	VertexBuffer getVertexBuffer() { return { m_vertexBuffer, static_cast<unsigned int>(m_vertices.size()), m_indexBuffer, static_cast<unsigned int>(m_indices.size()) }; }

private:
    // Vertex
	std::vector<T> m_vertices = {};
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

		createBuffer(device, physicalDevice, size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffer, m_vertexBufferMemory);

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

		createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBuffer, m_indexBufferMemory);

		copyBuffer(device, commandPool, graphicsQueue, stagingBuffer, m_indexBuffer, bufferSize);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}
};