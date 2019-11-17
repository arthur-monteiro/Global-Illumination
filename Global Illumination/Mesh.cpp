#include "Mesh.h"

MeshBase::~MeshBase() {

}

void MeshBase::createVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, void *data)
{
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(device, physicalDevice, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* tData;
    vkMapMemory(device, stagingBufferMemory, 0, size, 0, &tData);
    std::memcpy(tData, data, (size_t)size);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(device, physicalDevice, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffer, m_vertexBufferMemory);

    vk->copyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

    vkDestroyBuffer(vk->getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(vk->getDevice(), stagingBufferMemory, nullptr);
}
