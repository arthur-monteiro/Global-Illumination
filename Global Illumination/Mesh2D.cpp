#include "Mesh2D.h"

void Mesh2D::loadFromVertices(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::vector<Vertex2D> vertices, std::vector<uint32_t> indices)
{
    m_vertices = vertices;
    m_indices = indices;

	createVertexBuffer(device, physicalDevice, commandPool, graphicsQueue, sizeof(m_vertices[0]) * m_vertices.size(), m_vertices.data());
	createIndexBuffer(device, physicalDevice, commandPool, graphicsQueue);
}

void Mesh2D::cleanup(VkDevice device)
{

}