#include "Model2D.h"

Model2D::~Model2D()
{
}

int Model2D::addMeshFromVertices(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, std::vector<Vertex2D> vertices, std::vector<uint32_t> indices)
{
	Mesh<Vertex2D> mesh;
	mesh.loadFromVertices(device, physicalDevice, commandPool, graphicsQueue, std::move(vertices), std::move(indices));

	m_meshes.push_back(mesh);

	return m_meshes.size() - 1;
}
