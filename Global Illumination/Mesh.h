#pragma once

#include "VulkanHelper.h"

#include <cstring>

class MeshBase
{
public:
    virtual ~MeshBase();

protected:
    // Vertex
    VkBuffer m_vertexBuffer;
    VkDeviceMemory m_vertexBufferMemory;

    // Indices
    std::vector<uint32_t> m_indices;
    VkBuffer m_indexBuffer;
    VkDeviceMemory m_indexBufferMemory;

protected:
    static void createVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, void* data);
};