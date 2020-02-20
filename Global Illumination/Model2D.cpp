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

void Model2D::cleanup(VkDevice device)
{
	for (int i(0); i < m_meshes.size(); ++i)
		m_meshes[i].cleanup(device);
}

std::vector<VertexBuffer> Model2D::getVertexBuffers()
{
	std::vector<VertexBuffer> vertexBuffers;

	for (int i(0); i < m_meshes.size(); ++i)
	{
		vertexBuffers.push_back(m_meshes[i].getVertexBuffer());
	}

	return vertexBuffers;
}
